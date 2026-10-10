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

#include <cctype>
#include <map>
#include <regex>
#include <set>
#include <string>
#include "javagenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

JavaGenerator::wire_t JavaGenerator::java_wire(const std::string& type)
{
    static const std::map<std::string, wire_t> wires = {
        { "char",          { "byte",   "",       8 } },
        { "unsigned char", { "byte",   "",       8 } },
        { "uint8_t",       { "byte",   "",       8 } },
        { "int8_t",        { "byte",   "",       8 } },
        { "uint16_t",      { "short",  "Short", 16 } },
        { "int16_t",       { "short",  "Short", 16 } },
        { "uint32_t",      { "int",    "Int",   32 } },
        { "int32_t",       { "int",    "Int",   32 } },
        { "uint64_t",      { "long",   "Long",  64 } },
        { "int64_t",       { "long",   "Long",  64 } },
        { "float",         { "float",  "Float", 32 } },
        { "double",        { "double", "Double", 64 } },
    };

    auto it = wires.find(type);
    return it == wires.end() ? wire_t{ "", "", 0 } : it->second;
}

std::string JavaGenerator::java_type(const std::string& type)
{
    std::string mapped = lang_spec_.get_value_string(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string JavaGenerator::java_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return lang_spec_.get_value_string("big_endian_str");
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return lang_spec_.get_value_string("little_endian_str");
    return lang_spec_.get_value_string("native_endian_str");
}

// True for unsigned members held in a wider Java type than goes on the wire.
bool JavaGenerator::is_widened(const std::string& type)
{
    const wire_t wire = java_wire(type);
    return wire.bits != 0 && java_type(type) != wire.type;
}

std::string JavaGenerator::subst(std::string template_str, const member_desc_t& member)
{
    static const std::map<std::string, std::string> boxes = {
        { "byte", "Byte" }, { "short", "Short" }, { "int", "Integer" },
    };
    static const std::map<unsigned int, std::string> masks = {
        { 8, "0xFF" }, { 16, "0xFFFF" }, { 32, "0xFFFFFFFFL" },
    };

    const std::string& type = std::get<member_type>(member);
    const wire_t wire = java_wire(type);
    const std::string jtype = java_type(type);

    // e.g. Short.toUnsignedInt, Integer.toUnsignedLong
    std::string cast, widen, mask;
    if (is_widened(type))
    {
        std::string jtype_cap = jtype;
        jtype_cap[0] = static_cast<char>(std::toupper(jtype_cap[0]));

        cast = "(" + wire.type + ") ";
        widen = boxes.at(wire.type) + ".toUnsigned" + jtype_cap;
        mask = masks.at(wire.bits);
    }

    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("classname_substr")), classname_);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), jtype);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("endian_substr")), java_endian(std::get<member_endianness>(member)));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("accessor_substr")), wire.accessor);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("cast_substr")), cast);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("widen_substr")), widen);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("mask_substr")), mask);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("length_substr")), std::to_string(std::get<member_length>(member)));

    return template_str;
}

bool JavaGenerator::generate(int& errorcode)
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

bool JavaGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_package(errorcode);
    imp_imports(errorcode);
    imp_class(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool JavaGenerator::imp_banner(int& errorcode)
{
    bool success = gen_one_line("banner_str");

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

bool JavaGenerator::imp_package(int& errorcode)
{
    bool success = gen_one_line_subst("namespace_block", "namespace", "namespace_substr", 0);

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// Imports what the generated code uses, plus any extra imports the spec lists
// (e.g. for user-defined member types in another package).
bool JavaGenerator::imp_imports(int& errorcode)
{
    std::set<std::string> imports = { "java.nio.ByteBuffer", "java.nio.ByteOrder", "java.util.Objects" };

    for (auto& member : ordered_members_)
    {
        if (is_string_type(std::get<member_type>(member.second)))
        {
            imports.insert("java.util.Arrays");
        }
    }

    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
    for (auto& item : values)
    {
        if (item != "@NOTFOUND")
        {
            imports.insert(item);
        }
    }

    const std::string import_str = lang_spec_.get_value_string("import_str");
    const std::string include_substr = lang_spec_.get_value_string("include_substr");

    for (auto& import : imports)
    {
        ss_ << std::regex_replace(import_str, std::regex(include_substr), import);
    }

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return true;
}

bool JavaGenerator::imp_class(int& errorcode)
{
    const std::string separator = lang_spec_.get_value_string("block_separator_str");

    std::string body = imp_fields();
    body += separator + block("serialize_array_fcn_block", "");
    body += separator + block("serialize_fcn_block", imp_serialize());
    body += separator + block("deserialize_array_fcn_block", "");
    body += separator + block("deserialize_fcn_block", imp_deserialize());
    body += separator + block("size_fcn_block", imp_size());
    body += separator + block("equals_fcn_block", imp_equals());
    body += separator + block("hashcode_fcn_block", imp_hashcode());
    body += separator + block("tostring_fcn_block", imp_tostring());
    body += imp_get_set_functions();

    ss_ << block("class_block", body);

    return true;
}

std::string JavaGenerator::imp_fields()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "member_string_decl_str";
        else if (!is_native_type(type))
            key = "member_user_decl_str";
        else
            key = "member_decl_str";

        body += subst(lang_spec_.get_value_string(key), member.second);
    }

    return body;
}

std::string JavaGenerator::imp_serialize()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "serialize_string_mem_str";
        else if (!is_native_type(type))
            key = "serialize_user_mem_str";
        else if (java_wire(type).bits == 8)
            key = "serialize_byte_mem_str";
        else
            key = "serialize_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);
    }

    return body;
}

std::string JavaGenerator::imp_deserialize()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "deserialize_string_mem_str";
        else if (!is_native_type(type))
            key = "deserialize_user_mem_str";
        else if (java_wire(type).bits == 8)
            key = is_widened(type) ? "deserialize_widened_byte_mem_str" : "deserialize_byte_mem_str";
        else
            key = is_widened(type) ? "deserialize_widened_mem_str" : "deserialize_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);
    }

    return body;
}

std::string JavaGenerator::imp_size()
{
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
            body += std::to_string(java_wire(type).bits / 8);

        if (++i < ordered_members_.size())
        {
            body += separator;
        }
    }

    return body;
}

// Floats compare with Float.compare/Double.compare so equals agrees with
// hashCode (NaN equals itself, -0.0 differs from 0.0).
std::string JavaGenerator::imp_equals()
{
    const std::string separator = lang_spec_.get_value_string("equals_separator_str");

    std::string body;
    size_t i = 0;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "equals_string_mem_str";
        else if (!is_native_type(type))
            key = "equals_user_mem_str";
        else if (type == "float")
            key = "equals_float_mem_str";
        else if (type == "double")
            key = "equals_double_mem_str";
        else
            key = "equals_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);

        if (++i < ordered_members_.size())
        {
            body += separator;
        }
    }

    return body;
}

std::string JavaGenerator::imp_hashcode()
{
    const std::string separator = lang_spec_.get_value_string("hashcode_separator_str");

    std::string body;
    size_t i = 0;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key = is_string_type(type) ? "hashcode_string_mem_str" : "hashcode_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);

        if (++i < ordered_members_.size())
        {
            body += separator;
        }
    }

    return body;
}

std::string JavaGenerator::imp_tostring()
{
    std::string body;
    bool first = true;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key = is_string_type(type) ? "tostring_string_mem_str" : "tostring_mem_str";
        if (first)
        {
            key.insert(std::string("tostring_").size(), "first_");
        }

        body += subst(lang_spec_.get_value_string(key), member.second);
        first = false;
    }

    return body;
}

std::string JavaGenerator::imp_get_set_functions()
{
    const std::string separator = lang_spec_.get_value_string("block_separator_str");

    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string get_key = "get_str";
        std::string set_key = "set_str";

        if (is_string_type(type))
        {
            get_key = "get_string_str";
            set_key = "set_string_str";
        }
        else if (is_widened(type))
        {
            set_key = "set_widened_str";
        }

        body += separator + subst(lang_spec_.get_value_string(get_key), member.second);
        body += separator + subst(lang_spec_.get_value_string(set_key), member.second);
    }

    return body;
}

std::string JavaGenerator::block(const std::string block_id, const std::string& body)
{
    std::string block_str = lang_spec_.get_value_string(block_id);
    std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    std::string body_token_str = lang_spec_.get_value_string("body_token");

    block_str = std::regex_replace(block_str, std::regex(classname_substr), classname_);
    block_str = std::regex_replace(block_str, std::regex(body_token_str), body);

    return block_str;
}

}
