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

#include <algorithm>
#include <regex>
#include <map>
#include <set>
#include <string>
#include "gogenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

std::string GoGenerator::go_type(const std::string& type)
{
    std::string mapped = lang_spec_.get_value_string(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string GoGenerator::go_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return lang_spec_.get_value_string("big_endian_str");
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return lang_spec_.get_value_string("little_endian_str");
    return lang_spec_.get_value_string("native_endian_str");
}

// Width in bits of a native numeric type, taken from the trailing digits of
// its Go name (int8 -> 8, float64 -> 64). Zero for strings and user types.
unsigned int GoGenerator::go_bits(const std::string& type)
{
    if (is_string_type(type) || !is_native_type(type))
    {
        return 0;
    }

    std::smatch match;
    std::string mapped = go_type(type);
    if (std::regex_search(mapped, match, std::regex("([0-9]+)$")))
    {
        return static_cast<unsigned int>(std::stoul(match[1]));
    }
    return 0;
}

bool GoGenerator::is_float_type(const std::string& type)
{
    return type == "float" || type == "double";
}

std::string GoGenerator::subst(std::string template_str, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    unsigned int bits = go_bits(type);
    unsigned int wire_size = is_string_type(type) ? std::get<member_length>(member) : bits / 8;

    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("structname_substr")), classname_);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), go_type(type));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("endian_substr")), go_endian(std::get<member_endianness>(member)));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("bits_substr")), std::to_string(bits));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("length_substr")), std::to_string(std::get<member_length>(member)));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("size_substr")), std::to_string(wire_size));

    return template_str;
}

bool GoGenerator::generate(int& errorcode)
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

bool GoGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_package(errorcode);
    imp_imports(errorcode);
    imp_struct(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_equals(errorcode);
    imp_size(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);

    // gofmt ends a file with exactly one newline.
    std::string body = ss_.str();
    body.erase(body.find_last_not_of('\n') + 1);
    body += '\n';

    imp_string.append(body);

    return true;
}

bool GoGenerator::imp_banner(int& errorcode)
{
    bool success = gen_one_line("banner_str");

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

bool GoGenerator::imp_package(int& errorcode)
{
    bool success = gen_one_line_subst("namespace_block", "namespace", "namespace_substr", 0);

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// Go rejects unused imports, so import only the packages the members need,
// plus any extra packages the spec lists (e.g. for user-defined member types).
bool GoGenerator::imp_imports(int& errorcode)
{
    return do_block("import_block", [this]()->std::string {
        std::set<std::string> packages = { "fmt" };

        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);

            if (go_bits(type) > 8)
            {
                packages.insert("encoding/binary");
            }
            if (is_float_type(type))
            {
                packages.insert("math");
            }
        }

        const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
        for (auto& item : values)
        {
            if (item != "@NOTFOUND")
            {
                packages.insert(item);
            }
        }

        const std::string import_str = lang_spec_.get_value_string("import_str");
        const std::string include_substr = lang_spec_.get_value_string("include_substr");

        std::string body;
        for (auto& package : packages)
        {
            body += std::regex_replace(import_str, std::regex(include_substr), package);
        }

        return body;
    });
}

bool GoGenerator::imp_struct(int& errorcode)
{
    return do_block("struct_block", [this]()->std::string {
        // gofmt aligns field types in a column, so pad the names to match.
        size_t width = 0;
        for (auto& member : ordered_members_)
        {
            width = std::max(width, std::get<member_name>(member.second).size());
        }

        std::string body;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);
            std::string key = is_string_type(type) ? "member_string_decl_str" : "member_decl_str";

            member_desc_t padded = member.second;
            std::get<member_name>(padded).resize(width, ' ');

            body += subst(lang_spec_.get_value_string(key), padded);
        }

        return body;
    });
}

bool GoGenerator::imp_serialize(int& errorcode)
{
    return do_block("serialize_fcn_block", [this]()->std::string {
        std::string body;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);
            std::string key;

            if (is_string_type(type))
                key = "serialize_string_mem_str";
            else if (!is_native_type(type))
                key = "serialize_user_mem_str";
            else if (go_bits(type) == 8)
                key = "serialize_byte_mem_str";
            else if (is_float_type(type))
                key = "serialize_float_mem_str";
            else
                key = "serialize_int_mem_str";

            body += subst(lang_spec_.get_value_string(key), member.second);
        }

        return body;
    });
}

bool GoGenerator::imp_deserialize(int& errorcode)
{
    return do_block("deserialize_fcn_block", [this]()->std::string {
        std::string body;
        size_t i = 0;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);
            std::string key;
            std::string advance_key = "deserialize_advance_str";

            if (is_string_type(type))
                key = "deserialize_string_mem_str";
            else if (!is_native_type(type))
            {
                key = "deserialize_user_mem_str";
                advance_key = "deserialize_user_advance_str";
            }
            else if (go_bits(type) == 8)
                key = "deserialize_byte_mem_str";
            else if (is_float_type(type))
                key = "deserialize_float_mem_str";
            else
                key = "deserialize_int_mem_str";

            body += subst(lang_spec_.get_value_string(key), member.second);

            // Nothing reads past the last member, so don't advance after it.
            if (++i < ordered_members_.size())
            {
                body += subst(lang_spec_.get_value_string(advance_key), member.second);
            }
        }

        return body;
    });
}

bool GoGenerator::imp_equals(int& errorcode)
{
    return do_block("equals_fcn_block", [this]()->std::string {
        const std::string separator = lang_spec_.get_value_string("equals_separator_str");

        std::string body;
        size_t i = 0;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);
            std::string key = is_native_type(type) ? "equals_mem_str" : "equals_user_mem_str";

            body += subst(lang_spec_.get_value_string(key), member.second);

            if (++i < ordered_members_.size())
            {
                body += separator;
            }
        }

        return body;
    });
}

bool GoGenerator::imp_size(int& errorcode)
{
    return do_block("size_fcn_block", [this]()->std::string {
        const std::string separator = lang_spec_.get_value_string("size_separator_str");

        std::string body;
        size_t i = 0;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);

            if (is_string_type(type))
                body += std::to_string(std::get<member_length>(member.second));
            else if (!is_native_type(type))
                body += subst(lang_spec_.get_value_string("size_user_mem_str"), member.second);
            else
                body += std::to_string(go_bits(type) / 8);

            if (++i < ordered_members_.size())
            {
                body += separator;
            }
        }

        return body;
    });
}

bool GoGenerator::imp_get_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key = is_string_type(type) ? "get_string_str" : "get_str";

        ss_ << subst(lang_spec_.get_value_string(key), member.second);
        ss_ << lang_spec_.get_value_string("block_separator_str");
    }

    return true;
}

bool GoGenerator::imp_set_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key = is_string_type(type) ? "set_string_str" : "set_str";

        ss_ << subst(lang_spec_.get_value_string(key), member.second);
        ss_ << lang_spec_.get_value_string("block_separator_str");
    }

    return true;
}

bool GoGenerator::do_block(const std::string block_id, std::function<std::string()> builder)
{
    bool success = true;
    std::string block_str = lang_spec_.get_value_string(block_id);
    std::string structname_substr = lang_spec_.get_value_string("structname_substr");
    std::string body_token_str = lang_spec_.get_value_string("body_token");

    std::string new_body_str = builder();

    block_str = std::regex_replace(block_str, std::regex(structname_substr), classname_);
    block_str = std::regex_replace(block_str, std::regex(body_token_str), new_body_str);

    ss_ << block_str << lang_spec_.get_value_string("block_separator_str");

    return success;
}

}
