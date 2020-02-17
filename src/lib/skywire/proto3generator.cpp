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
#include "proto3generator.h"

namespace skyloupe
{

bool Proto3Generator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

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

bool Proto3Generator::do_header(int& errorcode)
{
    return true;
}

bool Proto3Generator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];
    imp_b0(errorcode);
    imp_b1(errorcode);
    imp_b2(errorcode);
    imp_b3(errorcode);
    imp_b4(errorcode);
    imp_b5(errorcode);
    imp_string.append(ss_.str());

    return true;
}

bool Proto3Generator::imp_b0(int& errorcode)
{
    bool success =  gen_one_line("banner_str");

    ss_ << msg_spec_.get_value_string("block_separator_str", "\n");

    return success;
}

bool Proto3Generator::imp_b1(int& errorcode)
{
    bool success =  gen_one_line("syntax_str");

    ss_ << msg_spec_.get_value_string("block_separator_str", "\n");

    return success;
}

bool Proto3Generator::imp_b2(int& errorcode)
{
    bool success = gen_one_line_subst("namespace_str", "namespace", "namespace_substr", 0);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    ss_ << msg_spec_.get_value_string("block_separator_str", "\n");

    return success;
}

bool Proto3Generator::imp_b3(int& errorcode)
{
    return true; //option
}

bool Proto3Generator::imp_b4(int& errorcode)
{
   return true;  //imports
}

bool Proto3Generator::imp_b5(int& errorcode)
{
    return do_block("message_block", [this]()->std::string {
        std::string member_substr = lang_spec_.get_value_string("member_substr");
        std::string type_substr = lang_spec_.get_value_string("type_substr");
        std::string index_substr = lang_spec_.get_value_string("index_substr");

        std::string body;
        for (auto& member : ordered_members_)
        {
            std::string template_str;
            template_str = lang_spec_.get_value_string("member_decl_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
            template_str = std::regex_replace(template_str, std::regex(type_substr), lang_spec_.get_value_string(std::get<member_type>(member.second)));
            template_str = std::regex_replace(template_str, std::regex(index_substr), std::to_string(member.first));

            body += template_str;
        }

        return body;
    });
}

bool Proto3Generator::do_block(const std::string block_id, std::function<std::string()> builder)
{
    bool success = true;
    std::string block_str = lang_spec_.get_value_string(block_id);
    std::string structname_substr = lang_spec_.get_value_string("structname_substr");
    std::string body_token_str = lang_spec_.get_value_string("body_token");

    std::string new_body_str = builder();

    block_str = std::regex_replace(block_str, std::regex(structname_substr), classname_);
    block_str = std::regex_replace(block_str, std::regex(body_token_str), new_body_str);

    ss_ << block_str;

    return success;
}

}