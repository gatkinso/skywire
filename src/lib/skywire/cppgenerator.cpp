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

#include <sstream>
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
    hdr_sentry(header_string, errorcode);
    hdr_includes(header_string, errorcode);
    hdr_namespace(header_string, errorcode);
    hdr_classname(header_string, errorcode);
    hdr_public(header_string, errorcode);
    hdr_cons(header_string, errorcode);
    hdr_des(header_string, errorcode);
    hdr_serialize(header_string, errorcode);
    hdr_deserialize(header_string, errorcode);
    hdr_size(header_string, errorcode);
    hdr_get_functions(header_string, errorcode);
    hdr_set_functions(header_string, errorcode);
    hdr_private(header_string, errorcode);
    hdr_members(header_string, errorcode);
    hdr_classend(header_string, errorcode);
    hdr_namespaceend(header_string, errorcode);

    return true;
}

bool CppGenerator::do_implementation(int& errorcode)
{
    return true;
}

bool CppGenerator::hdr_banner(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "banner_str");
}

bool CppGenerator::hdr_sentry(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "sentry_str");
}

bool CppGenerator::hdr_includes(std::string& str, int& errorcode)
{
    std::vector<std::string> values = msg_stencil_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
        return false;

    std::stringstream ss;
    for (auto& item : values)
    {
        ss << "#include " << item << std::endl;
    }

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_namespace(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line_subst(ss, str, "namespace_str", "namespace", "namespace_substr", 0);
}

bool CppGenerator::hdr_classname(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line_subst(ss, str, "classname_str", "classname", "classname_substr", 1);
}

bool CppGenerator::hdr_public(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return  gen_one_line(ss, str, "public_str", 1);
}

bool CppGenerator::hdr_private(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "private_str", 1);
}

bool CppGenerator::hdr_cons(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line_subst(ss, str, "cons_str", "classname", "classname_substr", 2);
}

bool CppGenerator::hdr_des(std::string& str, int& errorcode)
{
    std::stringstream ss;
    gen_one_line_subst(ss, str, "des_str", "classname", "classname_substr", 2);

    ss.str("");
    ss << std::endl;

    str.append(ss.str());

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
    std::stringstream ss;
    return gen_one_line(ss, str, "serialize_str", 2);
}

bool CppGenerator::hdr_deserialize(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "deserialize_str", 2);
}

bool CppGenerator::hdr_size(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "size_str", 2);
}

bool CppGenerator::hdr_members(std::string& str, int& errorcode)
{
    return inorder_members(str, errorcode);
}

bool CppGenerator::hdr_classend(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "endclass_str", 1);
}

bool CppGenerator::hdr_namespaceend(std::string& str, int& errorcode)
{
    std::stringstream ss;
    return gen_one_line(ss, str, "endnamespace_str");
}

bool CppGenerator::insert_indents(std::stringstream& ss, std::string& str, unsigned int indents)
{
    std::string indent_str = lang_stencil_.get_value_string("indent_str");

    for (unsigned int i = 0; i < indents; i++)
    {
        ss << indent_str;
    }

    return true;
}

bool CppGenerator::gen_one_line(std::stringstream& ss, std::string& str, const std::string& key, unsigned int indents)
{
    std::string value_str = lang_stencil_.get_value_string(key);

    insert_indents(ss, str, indents);

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::gen_one_line_subst(std::stringstream& ss, std::string& str, const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents)
{
    std::string template_str = lang_stencil_.get_value_string(template_key);
    std::string subst_str = msg_stencil_.get_value_string(subst_key);
    std::string target_str = lang_stencil_.get_value_string(target_key);

    insert_indents(ss, str, indents);

    template_str = std::regex_replace(template_str, std::regex(target_str), subst_str);

    ss << template_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::inorder_functions(std::string& str, const std::string& funct_str, int& errorcode)
{
    std::string type_substr = lang_stencil_.get_value_string("type_substr");
    std::string member_substr = lang_stencil_.get_value_string("member_substr");

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        return false;
    }

    std::stringstream ss;
    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        std::string template_str = lang_stencil_.get_value_string(funct_str);
        insert_indents(ss, str, 2);
        template_str = std::regex_replace(template_str, std::regex(type_substr), member.second.first);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);
        ss << template_str << std::endl;
    }

    ss << std::endl;
    str.append(ss.str());

    return true;
}

bool CppGenerator::inorder_members(std::string& str, int& errorcode)
{
    if (msg_stencil_.get_proto().members_size() == 0)
    {
        return false;
    }

    std::stringstream ss;

    std::map< uint32_t, std::pair<std::string, std::string> > ordered_members;
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members[member.first] = mempair;
    }

    for (auto& member : ordered_members)
    {
        insert_indents(ss, str, 2);
        ss << member.second.first << " " << member.second.second << ";" << std::endl;
    }

    str.append(ss.str());

    return true;
}

}