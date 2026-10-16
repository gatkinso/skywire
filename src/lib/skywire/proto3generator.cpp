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

#include <iostream>
#include <regex>
#include <string>
#include "proto3generator.h"
#include "sw_endian.h"

namespace skyloupe
{

// A nested message's protobuf type is its own message name.
std::string Proto3Generator::proto_type(const std::string& type)
{
    if (is_user_type(type))
    {
        return type;
    }

    return lang_spec_.get_value_string(type);
}

// The skywire-only facts about a member (its C type, byte order, string
// length) don't exist in protobuf, so they ride along as a comment.
std::string Proto3Generator::skywire_comment(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);

    if (is_user_type(type))
    {
        return lang_spec_.get_value_string("nested_str");
    }

    if (is_string_type(type))
    {
        return "char[" + std::to_string(std::get<member_length>(member)) + "]";
    }

    if (type == "char" || type == "unsigned char" || type == "uint8_t" || type == "int8_t")
    {
        return type;
    }

    std::string endian_key = "native_endian_str";
    if (std::get<member_endianness>(member) == SW_BIG_ENDIAN_FLAG)
        endian_key = "big_endian_str";
    else if (std::get<member_endianness>(member) == SW_LITTLE_ENDIAN_FLAG)
        endian_key = "little_endian_str";

    return type + ", " + lang_spec_.get_value_string(endian_key);
}

std::string Proto3Generator::subst(std::string template_str, uint32_t tag, const member_desc_t& member)
{
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), proto_type(std::get<member_type>(member)));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("tag_substr")), std::to_string(tag));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("comment_substr")), skywire_comment(member));

    return template_str;
}

bool Proto3Generator::generate(int& errorcode)
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

bool Proto3Generator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_syntax(errorcode);
    imp_package(errorcode);
    imp_imports(errorcode);
    imp_message(errorcode);

    std::string body = ss_.str();
    body.erase(body.find_last_not_of('\n') + 1);
    body += '\n';

    imp_string.append(body);

    return true;
}

bool Proto3Generator::imp_banner(int& errorcode)
{
    bool success = gen_one_line("banner_str");

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

bool Proto3Generator::imp_syntax(int& errorcode)
{
    bool success = gen_one_line("syntax_str");

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// The namespace is optional; a C++-style one (a::b) becomes a protobuf
// package (a.b).
bool Proto3Generator::imp_package(int& errorcode)
{
    std::string name_space = msg_spec_.get_value_string("namespace", "");
    if (name_space.empty())
    {
        return true;
    }

    name_space = std::regex_replace(name_space, std::regex("::"), ".");

    std::string package_str = lang_spec_.get_value_string("package_str");
    ss_ << std::regex_replace(package_str, std::regex(lang_spec_.get_value_string("namespace_substr")), name_space);
    ss_ << lang_spec_.get_value_string("block_separator_str");

    return true;
}

// Each nested message type comes from its own generated .proto, in the same package.
bool Proto3Generator::imp_imports(int& errorcode)
{
    const std::set<std::string> types = user_types();
    if (types.empty())
    {
        return true;
    }

    for (auto& type : types)
    {
        std::string import_str = lang_spec_.get_value_string("import_str");
        ss_ << std::regex_replace(import_str, std::regex(lang_spec_.get_value_string("type_substr")), type);
    }
    ss_ << lang_spec_.get_value_string("block_separator_str");

    return true;
}

// Member tags are the protobuf field numbers (validate_spec keeps them legal).
bool Proto3Generator::imp_message(int& errorcode)
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        body += subst(lang_spec_.get_value_string("member_decl_str"), member.first, member.second);
    }

    std::string block_str = lang_spec_.get_value_string("message_block");
    block_str = std::regex_replace(block_str, std::regex(lang_spec_.get_value_string("structname_substr")), classname_);
    block_str = std::regex_replace(block_str, std::regex(lang_spec_.get_value_string("body_token")), body);

    ss_ << block_str;

    return true;
}

}
