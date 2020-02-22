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
#include <string>
#include "py3generator.h"

namespace skyloupe
{

bool Py3Generator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    if (false == do_header(errorcode))
    {
        errorcode |= SW_FILE_HEADER;
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

bool Py3Generator::do_header(int& errorcode)
{
    return true;
}

bool Py3Generator::do_implementation(int& errorcode)
{
    std::string& header_string = results_["implementation"];

    imp_banner(errorcode);
    imp_sentry(errorcode);
    imp_includes(errorcode);
    imp_namespace(errorcode);
    imp_classname(errorcode);
    imp_public(errorcode);
    imp_cons(errorcode);
    imp_des(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals(errorcode);
    imp_print(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool Py3Generator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool Py3Generator::imp_sentry(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_includes(int& errorcode)
{
    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_INCLUDES;
        return false;
    }

    for (auto& item : values)
    {
        std::string template_str = lang_spec_.get_value_string("include_str");
        std::string target_str = lang_spec_.get_value_string("include_substr");

        template_str = std::regex_replace(template_str, std::regex(target_str), item);

        ss_ << template_str;
    }

    ss_ << std::endl;

    return true;
}

bool Py3Generator::imp_namespace(int& errorcode)
{
    bool success = gen_one_line("size_dict");
    if (false == success)
    {
        errorcode |= SW_LANG_RUNTIME;
        return success;
    }

    ss_ << std::endl;
    return success;
}

bool Py3Generator::imp_classname(int& errorcode)
{
    bool success = gen_one_line_subst("classname_str", "classname", "classname_substr", 0);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    return success;
}

bool Py3Generator::imp_public(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_private(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_cons(int& errorcode)
{
    bool success = gen_one_line("cons_str", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_IMP_CONS;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        template_str = lang_spec_.get_value_string("cons_member_init_str");
        
        insert_indents(2);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        ss_ << template_str << std::endl;
    }

    gen_one_line("endfunction_str");

    return true;
}

bool Py3Generator::imp_des(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_get_functions(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_set_functions(int& errorcode)
{
    return true;
}

bool Py3Generator::imp_serialize(int& errorcode)
{
    if (false == gen_one_line("serialize_fcn_str", 1))
    {
        errorcode |= SW_LANG_C_IMP_SERIALIZE;
        return false;
    }

    if (false == gen_one_line("serialize_start", 2))
    {
        errorcode |= SW_LANG_C_IMP_SERIALIZE;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_IMP_SERIALIZE;
        return false;
    }

    std::string format_str("\"");
    for (auto& member : ordered_members_)
    {
        format_str.append(lang_spec_.get_value_string(std::get<member_type>(member.second)));
    }
    format_str.append("\", \\\n");
    ss_ << format_str;

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        template_str = lang_spec_.get_value_string("serialize_member_str");
        insert_indents(2);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        ss_ << template_str << " \\" << std::endl;
    }

    insert_indents(2);
    ss_ << ")\n";

    gen_one_line("endfunction_str");

    return true;
}

bool Py3Generator::imp_deserialize(int& errorcode)
{
    if (false == gen_one_line("deserialize_fcn_str", 1))
    {
        errorcode |= SW_LANG_C_IMP_DESERIALIZE;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string index_substr = lang_spec_.get_value_string("index_substr");
    const std::string format_substr = lang_spec_.get_value_string("format_substr");

    if (msg_spec_.get_proto().members_size() == 0 || 
        index_substr == "@NOTFOUND" ||
        format_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_IMP_DESERIALIZE;
        return false;
    }

    std::string format_str("\"");
    for (auto& member : ordered_members_)
    {
        format_str.append(lang_spec_.get_value_string(std::get<member_type>(member.second)));
    }
    format_str.append("\"");

    std::string template_str;
    template_str = lang_spec_.get_value_string("deserialize_start");
    insert_indents(2);
    template_str = std::regex_replace(template_str, std::regex(format_substr), format_str);
    ss_ << template_str;

    int i = 0;
    for (auto& member : ordered_members_)
    {
        template_str = lang_spec_.get_value_string("deserialize_member_str");
        insert_indents(2);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        template_str = std::regex_replace(template_str, std::regex(index_substr), std::to_string(i++));

        ss_ << template_str;
    }

    gen_one_line("endfunction_str");

    return true;
}

bool Py3Generator::imp_size(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");

    if (msg_spec_.get_proto().members_size() == 0 || 
        member_substr == "@NOTFOUND" ||
        type_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_IMP_SIZE;
        return false;
    }

    if (false == gen_one_line("size_fcn_str", 1))
    {
        errorcode |= SW_LANG_C_IMP_SIZE;
        return false;
    }

    std::string template_str;
    template_str = lang_spec_.get_value_string("size_start");
    insert_indents(2);
    ss_ << template_str;

    int i = 0;
    for (auto& member : ordered_members_)
    {
        template_str = lang_spec_.get_value_string("size_mem_str");
        template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << " + \\\n";
            insert_indents(2);
        }
    }

    gen_one_line("endfunction_str");
    gen_one_line("endfunction_str");

    return true;
}

bool Py3Generator::imp_equals(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");

    if (msg_spec_.get_proto().members_size() == 0 || 
        member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_IMP_EQUALS;
        return false;
    }

    if (false == gen_one_line("equals_fcn_str", 1))
    {
        errorcode |= SW_LANG_C_IMP_EQUALS;
        return false;
    }

    std::string template_str;
    template_str = lang_spec_.get_value_string("equals_start");
    insert_indents(2);
    ss_ << template_str;

    int i = 0;
    for (auto& member : ordered_members_)
    {
        template_str = lang_spec_.get_value_string("equals_mem_str");
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << " and \\\n";
            insert_indents(2);
        }
    }

    gen_one_line("endfunction_str");
    gen_one_line("endfunction_str");

    return true;
}

bool Py3Generator::imp_print(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");

    if (msg_spec_.get_proto().members_size() == 0 || 
        member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_IMP_PRINT;
        return false;
    }

    if (false == gen_one_line("print_fcn_str", 1))
    {
        errorcode |= SW_LANG_IMP_PRINT;
        return false;
    }

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_spec_.get_value_string("print_mem_str");
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        insert_indents(2);
        ss_ << template_str;
    }

    gen_one_line("endfunction_str");

    return true;
}

}
