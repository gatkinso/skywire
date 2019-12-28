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

Generator::Generator(Stencil& stencil) 
: msg_stencil_(stencil) 
, results_()
{
    for (auto& member : msg_stencil_.get_proto().members())
    {
        member_desc_t memtuple = std::make_tuple(member.second.type(), member.second.name(), member.second.endian());
        std::pair<uint32_t, member_desc_t> mempair = std::make_pair(member.first, memtuple);
        ordered_members_.insert(mempair);
    }
}

bool Generator::is_native_type(const std::string& type)
{
    if (native_types_.end() == native_types_.find(type))
    {
        return false;
    }

    return true;
}

bool Generator::set_msg_settings(const std::string pathname)
{
    msg_stencil_.clear();

    return msg_stencil_.read_file(pathname);
}

bool Generator::set_lang_settings(const std::string pathname)
{
    lang_stencil_.clear();

    bool success =  lang_stencil_.read_file(pathname);

    std::vector<std::string> native_type = lang_stencil_.get_value("native_types");
    if (native_type[0] != "@NOTFOUND")
    {
        for (const auto& item : native_type)
        {
            native_types_.insert(item);
        }
    }

    return success;
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
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        insert_indents(indents);
        ss_ << std::get<member_type>(member.second) << " " << std::get<member_name>(member.second) << ";" << std::endl;
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
        errorcode |= SW_LANG_NOMEMBERS;
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
        template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        ss_ << template_str;
    }

    return true;
}

}