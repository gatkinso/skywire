//
//   Copyright (C) 2017 Skyloupe, LLC
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
#include "generator.h"

namespace skyloupe
{

Generator::Generator(Stencil& stencil) : msg_stencil_(stencil) 
{
    for (auto& member : msg_stencil_.get_proto().members())
    {
        std::pair<std::string, std::string> mempair = std::make_pair(member.second.type(), member.second.name());
        ordered_members_[member.first] = mempair;
    }
}

bool Generator::set_msg_settings(const std::string pathname)
{
    msg_stencil_.clear();

    return msg_stencil_.read_file(pathname);
}

bool Generator::set_lang_settings(const std::string pathname)
{
    lang_stencil_.clear();

    return lang_stencil_.read_file(pathname);
}

void Generator::cr()
{
    ss_ << std::endl;
}

void Generator::insert_char(const char c)
{
    ss_ << c;
}

bool Generator::insert_indents(unsigned int indents)
{
    std::string indent_str = lang_stencil_.get_value_string("indent_str");

    for (unsigned int i = 0; i < indents; i++)
    {
        ss_ << indent_str;
    }

    return true;
}

bool Generator::gen_one_line(const std::string& key, unsigned int indents)
{
    std::string value_str = lang_stencil_.get_value_string(key);

    insert_indents(indents);

    ss_ << value_str;

    return true;
}

bool Generator::gen_one_line_subst(const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents)
{
    std::string template_str = lang_stencil_.get_value_string(template_key);
    std::string subst_str = msg_stencil_.get_value_string(subst_key);
    std::string target_str = lang_stencil_.get_value_string(target_key);

    insert_indents(indents);

    template_str = std::regex_replace(template_str, std::regex(target_str), subst_str);

    ss_ << template_str;

    return true;
}

bool Generator::inorder_members(int& errorcode, unsigned int indents)
{
    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        insert_indents(indents);
        ss_ << member.second.first << " " << member.second.second << ";" << std::endl;
    }

    return true;
}

bool Generator::inorder_functions(const std::string& funct_str, int& errorcode, unsigned int indents, bool prepend_classname)
{
    std::string type_substr = lang_stencil_.get_value_string("type_substr");
    std::string member_substr = lang_stencil_.get_value_string("member_substr");
    std::string class_substr("");
    if (prepend_classname == true)
    {
        class_substr = lang_stencil_.get_value_string("classname_substr");
    }

    if (msg_stencil_.get_proto().members_size() == 0)
    {
        errorcode = SW_LANGCPPNOMEMBERS;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_stencil_.get_value_string(funct_str);
        insert_indents(indents);
        if (prepend_classname == true)
        {
            template_str = 
                std::regex_replace(template_str, std::regex(class_substr), msg_stencil_.get_value_string("classname"));
        }
        template_str = std::regex_replace(template_str, std::regex(type_substr), member.second.first);
        template_str = std::regex_replace(template_str, std::regex(member_substr), member.second.second);

        ss_ << template_str;
    }

    return true;
}
}