//
//   Copyright (C) 2019 Skyloupe, LLC
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
#include <string>
#include "swiftgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

std::string SwiftGenerator::swift_type(const std::string& type)
{
    std::string mapped = lang_spec_.get_value_string(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string SwiftGenerator::swift_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return ".big";
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return ".little";
    return ".native";
}

bool SwiftGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    ss_.str("");

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool SwiftGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_imports(errorcode);
    imp_classname(errorcode);
    imp_members(errorcode);
    imp_cons(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);
    imp_classend(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool SwiftGenerator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool SwiftGenerator::imp_imports(int& errorcode)
{
    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");

    if (values.empty() || values[0] == "@NOTFOUND")
    {
        return true;
    }

    const std::string import_str = lang_spec_.get_value_string("import_str");
    const std::string import_substr = lang_spec_.get_value_string("import_substr");

    for (auto& item : values)
    {
        std::string template_str = std::regex_replace(import_str, std::regex(import_substr), item);
        ss_ << template_str;
    }

    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_classname(int& errorcode)
{
    std::string template_str = lang_spec_.get_value_string("classname_str");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    template_str = std::regex_replace(template_str, std::regex(classname_substr), classname_);
    ss_ << template_str;

    return true;
}

bool SwiftGenerator::imp_members(int& errorcode)
{
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("member_string_decl_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string(is_user_type(std::get<member_type>(member.second)) ? "member_user_decl_str" : "member_decl_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), swift_type(std::get<member_type>(member.second)));
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_cons(int& errorcode)
{
    insert_indents(1);
    gen_one_line("cons_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_serialize(int& errorcode)
{
    insert_indents(1);
    gen_one_line("serialize_str");

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        std::string template_str;
        insert_indents(2);

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("serialize_string_mem_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("serialize_native_mem_str");
            template_str = std::regex_replace(template_str, std::regex(endian_substr), swift_endian(std::get<member_endianness>(member.second)));
        }
        else
        {
            // A nested message uses its own members' byte orders.
            template_str = lang_spec_.get_value_string("serialize_user_mem_str");
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str << std::endl;
    }

    insert_indents(2);
    ss_ << "return true" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_deserialize(int& errorcode)
{
    insert_indents(1);
    gen_one_line("deserialize_str");

    // Reject a short stream before touching any field, like the Go, Java and Python generators.
    gen_one_line("deserialize_size_check_str", 2);

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        std::string template_str;
        insert_indents(2);

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_string_mem_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_native_mem_str");
            template_str = std::regex_replace(template_str, std::regex(endian_substr), swift_endian(std::get<member_endianness>(member.second)));
        }
        else
        {
            // A nested message uses its own members' byte orders.
            template_str = lang_spec_.get_value_string("deserialize_user_mem_str");
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str << std::endl;
    }

    insert_indents(2);
    ss_ << "return true" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_size(int& errorcode)
{
    insert_indents(1);
    gen_one_line("size_str");

    insert_indents(2);
    ss_ << "return (" << std::endl;

    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_string_mem_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_native_mem_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), swift_type(std::get<member_type>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("size_user_mem_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        }

        insert_indents(3);
        ss_ << template_str;

        if (i++ < static_cast<int>(ordered_members_.size()) - 1)
        {
            ss_ << " +";
        }

        ss_ << std::endl;
    }

    insert_indents(2);
    ss_ << ")" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_equals(int& errorcode)
{
    std::string template_str = lang_spec_.get_value_string("equals_str");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    template_str = std::regex_replace(template_str, std::regex(classname_substr), classname_);
    insert_indents(1);
    ss_ << template_str;

    insert_indents(2);
    ss_ << "return (" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string member_tmpl;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            member_tmpl = lang_spec_.get_value_string("equals_string_mem_str");
            member_tmpl = std::regex_replace(member_tmpl, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_user_type(std::get<member_type>(member.second)))
        {
            member_tmpl = lang_spec_.get_value_string("equals_user_mem_str");
        }
        else
        {
            member_tmpl = lang_spec_.get_value_string("equals_mem_str");
        }

        member_tmpl = std::regex_replace(member_tmpl, std::regex(member_substr), std::get<member_name>(member.second));
        insert_indents(3);
        ss_ << member_tmpl;

        if (i++ < static_cast<int>(ordered_members_.size()) - 1)
        {
            ss_ << " &&";
        }

        ss_ << std::endl;
    }

    insert_indents(2);
    ss_ << ")" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_get_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("get_string_str");
        }
        else
        {
            template_str = lang_spec_.get_value_string("get_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), swift_type(std::get<member_type>(member.second)));
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool SwiftGenerator::imp_set_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("set_string_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("set_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), swift_type(std::get<member_type>(member.second)));
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool SwiftGenerator::imp_classend(int& errorcode)
{
    return gen_one_line("endclass_str");
}

}
