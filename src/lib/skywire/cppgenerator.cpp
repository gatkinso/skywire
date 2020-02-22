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
#include "cppgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

bool CppGenerator::generate(int& errorcode)
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

bool CppGenerator::do_header(int& errorcode)
{
    std::string& header_string = results_["header"];

    hdr_banner(errorcode);
    hdr_sentry(errorcode);
    hdr_includes(errorcode);
    hdr_namespace(errorcode);
    hdr_classname(errorcode);
    hdr_public(errorcode);
    hdr_cons(errorcode);
    hdr_des(errorcode);
    hdr_serialize(errorcode);
    hdr_deserialize(errorcode);
    hdr_size(errorcode);
    hdr_equals(errorcode);
    hdr_get_functions(errorcode);
    hdr_set_functions(errorcode);
    hdr_private(errorcode);
    hdr_members(errorcode);
    hdr_classend(errorcode);
    hdr_namespaceend(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CppGenerator::do_implementation(int& errorcode)
{
    //For now generating C++ header only code...
    return true;
}

bool CppGenerator::hdr_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool CppGenerator::hdr_sentry(int& errorcode)
{
    return gen_one_line("sentry_str");
}

bool CppGenerator::hdr_includes(int& errorcode)
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

    return true;
}

bool CppGenerator::hdr_namespace(int& errorcode)
{
    bool success = gen_one_line_subst("namespace_str", "namespace", "namespace_substr", 0);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_NAMESPACE;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_classname(int& errorcode)
{
    bool success = gen_one_line_subst("classname_str", "classname", "classname_substr", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_public(int& errorcode)
{
    bool success = gen_one_line("public_str", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_PUBLIC;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_private(int& errorcode)
{
    bool success = gen_one_line("private_str", 1);
    
    if (false == success)
    {
        errorcode |= SW_LANG_CPP_PRIVATE;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_cons(int& errorcode)
{
    bool success = gen_one_line_subst("cons_str", "classname", "classname_substr", 2);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_CONSTRUCTOR;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_des(int& errorcode)
{
    bool success = gen_one_line_subst("des_str", "classname", "classname_substr", 2);
    
    if (false == success)
    {
        errorcode |= SW_LANG_CPP_DESTRUCTOR;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_get_functions(int& errorcode)
{
    bool success = inorder_functions("get_str", errorcode, 2);
    
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_GET;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_set_functions(int& errorcode)
{
    bool success = inorder_functions("set_str", errorcode, 2);
    
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_SET;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_serialize(int& errorcode)
{
    if (false == gen_one_line("serialize_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("serialize_native_mem_str");
        }
        else
        {
            template_str = lang_spec_.get_value_string("serialize_user_mem_str");
        }
        
        insert_indents(3);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        if (std::get<member_endianness>(member.second) == SW_BIG_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Big");
        else if (std::get<member_endianness>(member.second) == SW_LITTLE_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Little");
        else
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Native");

        ss_ << template_str << std::endl;
    }

    insert_indents(3);
    ss_ << "return true;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

bool CppGenerator::hdr_deserialize(int& errorcode)
{
    if (false == gen_one_line("deserialize_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_native_mem_str");
        }
        else
        {
            template_str = lang_spec_.get_value_string("deserialize_user_mem_str");
        }

        insert_indents(3);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        if (std::get<member_endianness>(member.second) == SW_BIG_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Big");
        else if (std::get<member_endianness>(member.second) == SW_LITTLE_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Little");
        else
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "skyloupe::Endianness::Native");
        ss_ << template_str << std::endl;
    }

    insert_indents(3);
    ss_ << "return true;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

bool CppGenerator::hdr_size(int& errorcode)
{
    if (false == gen_one_line("size_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SIZE;
        return false;
    }

    insert_indents(3);
    ss_ << "return(" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_SIZE;
        return false;
    }

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_native_mem_str");
        }
        else
        {
            template_str = lang_spec_.get_value_string("size_user_mem_str");
        }

        insert_indents(4);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_type>(member.second));
        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << " +";
        }

        ss_ << std::endl;
    }

    insert_indents(3);
    ss_ << ");" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

bool CppGenerator::hdr_equals(int& errorcode)
{
    if (false == gen_one_line_subst("equals_str", "classname", "classname_substr", 2))
    {
        errorcode |= SW_LANG_C_HDR_DESERIALIZE;
        return false;
    }

    const std::string equals_selfcheck_str = lang_spec_.get_value_string("equals_selfcheck_str");
    insert_indents(3);
    ss_ << equals_selfcheck_str;
    insert_indents(3);
    ss_ << "return(" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_EQUALS;
        return false;
    }

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_spec_.get_value_string("equals_mem_str");

        insert_indents(4);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << " &&";
        }

        ss_ << std::endl;
    }

    insert_indents(3);
    ss_ << ");\n";

    insert_indents(2);
    gen_one_line("endfunction_str");
    return true;
}

bool CppGenerator::hdr_members(int& errorcode)
{
    return inorder_members(errorcode, 2);
}

bool CppGenerator::hdr_classend(int& errorcode)
{
    return gen_one_line("endclass_str", 1);
}

bool CppGenerator::hdr_namespaceend(int& errorcode)
{
    return gen_one_line("endnamespace_str");
}

}
