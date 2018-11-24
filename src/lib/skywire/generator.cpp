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

void Generator::cr(std::string& str)
{
    ss_ << std::endl;
}

bool Generator::insert_indents(std::string& str, unsigned int indents)
{
    std::string indent_str = lang_stencil_.get_value_string("indent_str");

    for (unsigned int i = 0; i < indents; i++)
    {
        ss_ << indent_str;
    }

    return true;
}

bool Generator::gen_one_line(std::string& str, const std::string& key, unsigned int indents)
{
    std::string value_str = lang_stencil_.get_value_string(key);

    insert_indents(str, indents);

    ss_ << value_str << std::endl;

    return true;
}

bool Generator::gen_one_line_subst(std::string& str, const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents)
{
    std::string template_str = lang_stencil_.get_value_string(template_key);
    std::string subst_str = msg_stencil_.get_value_string(subst_key);
    std::string target_str = lang_stencil_.get_value_string(target_key);

    insert_indents(str, indents);

    template_str = std::regex_replace(template_str, std::regex(target_str), subst_str);

    ss_ << template_str << std::endl;

    return true;
}

}