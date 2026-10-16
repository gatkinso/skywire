//
//   Copyright (C) 2018 Skyloupe, LLC
//
//   Licensed under the Apache License, Version 2.0 (the "License").
//   You may not use this file except in compliance with the License.
//   A copy of the License is located at
//
//   https://www.apache.org/licenses/LICENSE-2.0.html
//
//   or in the "license" file accompanying this file. This file is distributed
//   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
//   express or implied. See the License for the specific language governing
//   permissions and limitations under the License.
//

#include <regex>
#include <map>
#include <set>
#include <string>
#include "py3generator.h"
#include "sw_endian.h"

namespace skyloupe
{

std::string Py3Generator::py_format(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);

    if (is_string_type(type))
    {
        return std::to_string(std::get<member_length>(member)) + "s";
    }

    // An explicit byte order also selects standard sizes and no alignment.
    std::string prefix;
    switch (std::get<member_endianness>(member))
    {
    case SW_BIG_ENDIAN_FLAG:    prefix = lang_spec_.get_value_string("big_endian_str"); break;
    case SW_LITTLE_ENDIAN_FLAG: prefix = lang_spec_.get_value_string("little_endian_str"); break;
    default:                    prefix = lang_spec_.get_value_string("native_endian_str"); break;
    }

    return prefix + lang_spec_.get_value_string(type);
}

unsigned int Py3Generator::py_size(const member_desc_t& member)
{
    static const std::map<std::string, unsigned int> sizes = {
        { "b", 1 }, { "B", 1 }, { "h", 2 }, { "H", 2 }, { "i", 4 }, { "I", 4 },
        { "q", 8 }, { "Q", 8 }, { "f", 4 }, { "d", 8 },
    };

    const std::string& type = std::get<member_type>(member);

    if (is_string_type(type))
    {
        return std::get<member_length>(member);
    }
    if (!is_native_type(type))
    {
        return 0;
    }

    auto it = sizes.find(lang_spec_.get_value_string(type));
    return it == sizes.end() ? 0 : it->second;
}

std::string Py3Generator::subst(std::string template_str, const member_desc_t& member)
{
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("classname_substr")), classname_);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), std::get<member_type>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("length_substr")), std::to_string(std::get<member_length>(member)));

    return template_str;
}

void Py3Generator::method(const std::string& def_key, const std::string& doc_key)
{
    ss_ << std::endl;
    insert_indents(1);
    ss_ << lang_spec_.get_value_string(def_key);

    if (!doc_key.empty())
    {
        line(lang_spec_.get_value_string(doc_key));
    }
}

void Py3Generator::line(const std::string& text, unsigned int indents)
{
    insert_indents(indents);
    ss_ << text;
}

bool Py3Generator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    ss_.str("");

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool Py3Generator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_includes(errorcode);
    imp_classname(errorcode);
    imp_cons(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals(errorcode);
    imp_dunders(errorcode);
    imp_print(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool Py3Generator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

// struct is always needed; the spec may list more modules.
bool Py3Generator::imp_includes(int& errorcode)
{
    std::set<std::string> modules = { "struct" };

    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
    for (auto& item : values)
    {
        if (item != "@NOTFOUND")
        {
            modules.insert(item);
        }
    }

    const std::string include_str = lang_spec_.get_value_string("include_str");
    const std::string include_substr = lang_spec_.get_value_string("include_substr");

    for (auto& module : modules)
    {
        ss_ << std::regex_replace(include_str, std::regex(include_substr), module);
    }

    // Each nested message type comes from its own generated module.
    const std::string user_import_str = lang_spec_.get_value_string("user_import_str");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");

    for (auto& type : user_types())
    {
        ss_ << std::regex_replace(user_import_str, std::regex(type_substr), type);
    }

    return true;
}

bool Py3Generator::imp_classname(int& errorcode)
{
    return gen_one_line_subst("classname_str", "classname", "classname_substr", 0);
}

bool Py3Generator::imp_cons(int& errorcode)
{
    method("cons_str");

    const std::string init_substr = lang_spec_.get_value_string("init_substr");

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string init_key;

        if (is_string_type(type))
            init_key = "init_string_str";
        else if (!is_native_type(type))
            init_key = "init_user_str";
        else if (type == "float" || type == "double")
            init_key = "init_float_str";
        else
            init_key = "init_int_str";

        std::string template_str = lang_spec_.get_value_string("cons_member_init_str");
        template_str = std::regex_replace(template_str, std::regex(init_substr), lang_spec_.get_value_string(init_key));
        line(subst(template_str, member.second));
    }

    return true;
}

bool Py3Generator::imp_serialize(int& errorcode)
{
    method("serialize_fcn_str", "serialize_doc_str");
    line(lang_spec_.get_value_string("serialize_start"));

    const std::string format_substr = lang_spec_.get_value_string("format_substr");

    for (auto& member : ordered_members_)
    {
        if (is_native_type(std::get<member_type>(member.second)))
        {
            std::string template_str = lang_spec_.get_value_string("serialize_member_str");
            template_str = std::regex_replace(template_str, std::regex(format_substr), py_format(member.second));
            line(subst(template_str, member.second), 3);
        }
        else
        {
            line(subst(lang_spec_.get_value_string("serialize_user_member_str"), member.second), 3);
        }
    }

    line(lang_spec_.get_value_string("serialize_end"));

    return true;
}

bool Py3Generator::imp_deserialize(int& errorcode)
{
    method("deserialize_fcn_str", "deserialize_doc_str");
    line(lang_spec_.get_value_string("deserialize_check_str"));

    std::string raise_str = lang_spec_.get_value_string("deserialize_raise_str");
    raise_str = std::regex_replace(raise_str, std::regex(lang_spec_.get_value_string("classname_substr")), classname_);
    line(raise_str, 3);

    const std::string format_substr = lang_spec_.get_value_string("format_substr");
    const std::string offset_substr = lang_spec_.get_value_string("offset_substr");

    // Offsets are constants until the first user-defined member, whose size
    // is only known at run time.
    unsigned int fixed_offset = 0;
    std::string dynamic_offset;

    for (auto& member : ordered_members_)
    {
        const std::string offset = std::to_string(fixed_offset) + dynamic_offset;
        std::string template_str;

        if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_member_str");
            template_str = std::regex_replace(template_str, std::regex(format_substr), py_format(member.second));
            fixed_offset += py_size(member.second);
        }
        else
        {
            template_str = lang_spec_.get_value_string("deserialize_user_member_str");
            dynamic_offset += " + " + subst(lang_spec_.get_value_string("size_user_mem_str"), member.second);
        }

        template_str = std::regex_replace(template_str, std::regex(offset_substr), offset);
        line(subst(template_str, member.second));
    }

    return true;
}

bool Py3Generator::imp_size(int& errorcode)
{
    method("size_fcn_str", "size_doc_str");

    const std::string separator = lang_spec_.get_value_string("size_separator_str");

    std::string body = lang_spec_.get_value_string("size_start");
    size_t i = 0;
    for (auto& member : ordered_members_)
    {
        if (is_native_type(std::get<member_type>(member.second)))
            body += std::to_string(py_size(member.second));
        else
            body += subst(lang_spec_.get_value_string("size_user_mem_str"), member.second);

        if (++i < ordered_members_.size())
        {
            body += separator;
        }
    }

    line(body + "\n");

    return true;
}

bool Py3Generator::imp_equals(int& errorcode)
{
    method("equals_fcn_str");
    line(lang_spec_.get_value_string("equals_start"));

    bool first = true;
    for (auto& member : ordered_members_)
    {
        std::string key = first ? "equals_first_mem_str" : "equals_mem_str";
        line(subst(lang_spec_.get_value_string(key), member.second), 3);
        first = false;
    }

    line(lang_spec_.get_value_string("equals_end"));

    return true;
}

bool Py3Generator::imp_dunders(int& errorcode)
{
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");

    method("eq_fcn_str");
    line(std::regex_replace(lang_spec_.get_value_string("eq_body_str"), std::regex(classname_substr), classname_));

    method("repr_fcn_str");
    line(std::regex_replace(lang_spec_.get_value_string("repr_body_str"), std::regex(classname_substr), classname_));

    return true;
}

bool Py3Generator::imp_print(int& errorcode)
{
    method("print_fcn_str");

    for (auto& member : ordered_members_)
    {
        line(subst(lang_spec_.get_value_string("print_mem_str"), member.second));
    }

    return true;
}

bool Py3Generator::imp_get_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        ss_ << std::endl;
        line(subst(lang_spec_.get_value_string("get_fcn_str"), member.second), 1);
        line(subst(lang_spec_.get_value_string("get_body_str"), member.second));
    }

    return true;
}

bool Py3Generator::imp_set_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string body_key;

        if (is_string_type(type))
            body_key = "set_string_body_str";
        else if (type == "float")
            body_key = "set_float_body_str";   // round to the float32 that goes on the wire
        else
            body_key = "set_body_str";

        ss_ << std::endl;
        line(subst(lang_spec_.get_value_string("set_fcn_str"), member.second), 1);
        line(subst(lang_spec_.get_value_string(body_key), member.second));
    }

    return true;
}

}
