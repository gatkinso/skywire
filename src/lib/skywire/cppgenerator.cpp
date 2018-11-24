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

namespace skyloupe
{

CppGenerator::CppGenerator(Stencil& stencil) : Generator(stencil) 
{

}

bool CppGenerator::generate(int& errorcode)
{
    std::cout << "Generating C++" << std::endl;
    errorcode = 0;

    results_.clear();

    if (false == do_header(errorcode)) return false;
    if (false == do_implementation(errorcode)) return false;

    return true;
}

bool CppGenerator::do_header(int& errorcode)
{
    results_.push_back("");

    std::string& header_string = results_[0];

    hdr_banner(header_string, errorcode);
    cr(header_string);
    hdr_sentry(header_string, errorcode);
    cr(header_string);
    hdr_includes(header_string, errorcode);
    cr(header_string);
    hdr_namespace(header_string, errorcode);
    hdr_classname(header_string, errorcode);
    hdr_public(header_string, errorcode);
    hdr_cons(header_string, errorcode);
    hdr_des(header_string, errorcode);
    cr(header_string);
    hdr_serialize(header_string, errorcode);
    hdr_deserialize(header_string, errorcode);
    hdr_size(header_string, errorcode);
    cr(header_string);
    hdr_get_functions(header_string, errorcode);
    cr(header_string);
    hdr_set_functions(header_string, errorcode);
    cr(header_string);
    hdr_private(header_string, errorcode);
    hdr_members(header_string, errorcode);
    hdr_classend(header_string, errorcode);
    hdr_namespaceend(header_string, errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CppGenerator::do_implementation(int& errorcode)
{
    //For now generating C++ header only code...
    return true;
}

bool CppGenerator::hdr_banner(std::string& str, int& errorcode)
{
    return gen_one_line(str, "banner_str");
}

bool CppGenerator::hdr_sentry(std::string& str, int& errorcode)
{
    return gen_one_line(str, "sentry_str");
}

bool CppGenerator::hdr_includes(std::string& str, int& errorcode)
{
    std::vector<std::string> values = msg_stencil_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
    {
        errorcode = SW_LANGCPPINCLUDES;
        return false;
    }

    for (auto& item : values)
    {
        ss_ << "#include " << item << std::endl;
    }

    return true;
}

bool CppGenerator::hdr_namespace(std::string& str, int& errorcode)
{
    return gen_one_line_subst(str, "namespace_str", "namespace", "namespace_substr", 0);
}

bool CppGenerator::hdr_classname(std::string& str, int& errorcode)
{
    return gen_one_line_subst(str, "classname_str", "classname", "classname_substr", 1);
}

bool CppGenerator::hdr_public(std::string& str, int& errorcode)
{
    return  gen_one_line(str, "public_str", 1);
}

bool CppGenerator::hdr_private(std::string& str, int& errorcode)
{
    return gen_one_line(str, "private_str", 1);
}

bool CppGenerator::hdr_cons(std::string& str, int& errorcode)
{
    return gen_one_line_subst(str, "cons_str", "classname", "classname_substr", 2);
}

bool CppGenerator::hdr_des(std::string& str, int& errorcode)
{
    gen_one_line_subst(str, "des_str", "classname", "classname_substr", 2);

    return true;
}

bool CppGenerator::hdr_get_functions(std::string& str, int& errorcode)
{
    return inorder_functions(str, "get_str", errorcode);
}

bool CppGenerator::hdr_set_functions(std::string& str, int& errorcode)
{
    return inorder_functions(str, "set_str", errorcode);
}

bool CppGenerator::hdr_serialize(std::string& str, int& errorcode)
{
    gen_one_line(str, "serialize_str", 2);

    std::string member_substr = lang_stencil_.get_value_string("member_substr");

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        std::string template_str = lang_stencil_.get_value_string("serialize_mem_str");
        insert_indents(str, 3);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);
        ss_ << template_str << std::endl;
    }

    insert_indents(str, 3);
    ss_ << "return true;" << std::endl;
    insert_indents(str, 2);
    gen_one_line(str, "endfunction_str");

    return true;
}

bool CppGenerator::hdr_deserialize(std::string& str, int& errorcode)
{
    gen_one_line(str, "deserialize_str", 2);

    std::string member_substr = lang_stencil_.get_value_string("member_substr");

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        std::string template_str = lang_stencil_.get_value_string("deserialize_mem_str");
        insert_indents(str, 3);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);
        ss_ << template_str << std::endl;
    }

    insert_indents(str, 3);
    ss_ << "return true;" << std::endl;
    insert_indents(str, 2);
    gen_one_line(str, "endfunction_str");

    return true;
}

bool CppGenerator::hdr_size(std::string& str, int& errorcode)
{
    gen_one_line(str, "size_str", 2);

    insert_indents(str, 3);
    ss_ << "return(" << std::endl;

    std::string member_substr = lang_stencil_.get_value_string("member_substr");

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    int i = 0;
    for (auto& member : ordered_members)
    {
        std::string template_str = lang_stencil_.get_value_string("size_mem_str");
        insert_indents(str, 4);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);
        ss_ << template_str;

        if (i++ < ordered_members.size() - 1)
        {
            ss_ << "+";
        }

        ss_ << std::endl;
    }

    insert_indents(str, 3);
    ss_ << ");" << std::endl;
    insert_indents(str, 2);
    gen_one_line(str, "endfunction_str");

    return true;
}

bool CppGenerator::hdr_members(std::string& str, int& errorcode)
{
    return inorder_members(str, errorcode);
}

bool CppGenerator::hdr_classend(std::string& str, int& errorcode)
{
    return gen_one_line(str, "endclass_str", 1);
}

bool CppGenerator::hdr_namespaceend(std::string& str, int& errorcode)
{
    return gen_one_line(str, "endnamespace_str");
}

bool CppGenerator::inorder_functions(std::string& str, const std::string& funct_str, int& errorcode)
{
    std::string type_substr = lang_stencil_.get_value_string("type_substr");
    std::string member_substr = lang_stencil_.get_value_string("member_substr");

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        std::string template_str = lang_stencil_.get_value_string(funct_str);
        insert_indents(str, 2);
        template_str = std::regex_replace(template_str, std::regex(type_substr), member.second.first);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);
        ss_ << template_str << std::endl;
    }

    return true;
}

bool CppGenerator::inorder_members(std::string& str, int& errorcode)
{
    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        insert_indents(str, 2);
        ss_ << member.second.first << " " << member.second.second << ";" << std::endl;
    }

    return true;
}

}