//
//   Copyright (C) 2026 Geoffrey Atkinson
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
#include <cctype>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include "naming.h"
#include "rustgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

namespace
{

// Keywords and reserved words, which a member name takes as a raw
// identifier (r#type).
const std::set<std::string> rust_keywords = {
    "abstract", "as", "async", "await", "become", "box", "break", "const", "continue", "do", "dyn",
    "else", "enum", "extern", "false", "final", "fn", "for", "gen", "if", "impl", "in", "let", "loop",
    "macro", "match", "mod", "move", "mut", "override", "priv", "pub", "ref", "return", "static",
    "struct", "trait", "true", "try", "type", "typeof", "unsafe", "unsized", "use", "virtual", "where",
    "while", "yield",
};

// Keywords that can't be raw identifiers, and _, which isn't an identifier.
const std::set<std::string> unusable_names = { "self", "Self", "super", "crate", "_" };

// What the generated code itself defines on a message: a member's accessor
// may not take one of these names. (An inherent default() would also hide
// Default::default from Self::default().)
const std::set<std::string> generated_names = {
    "SIZE", "size", "serialize", "serialize_into", "write_to", "deserialize", "from_bytes", "equals", "default",
};

bool has_case(char c)
{
    return std::isalpha(static_cast<unsigned char>(c)) != 0;
}

std::string trim_underscores(const std::string& name)
{
    size_t first = name.find_first_not_of('_');
    if (first == std::string::npos)
    {
        return "";
    }
    return name.substr(first, name.find_last_not_of('_') - first + 1);
}

bool spec_error(int& errorcode, const std::string& what)
{
    errorcode |= SW_SPEC_INVALID;
    std::cerr << "Invalid specification for Rust: " << what << std::endl;
    return false;
}

}

std::string RustGenerator::identifier(const std::string& name)
{
    return rust_keywords.count(name) != 0 ? "r#" + name : name;
}

std::string RustGenerator::module_name(const std::string& type)
{
    return snake_case(type);
}

// rustc's non_snake_case rule: no capitals, and no double underscore inside
// the name.
bool RustGenerator::is_snake_case(const std::string& name)
{
    const std::string trimmed = trim_underscores(name);
    return trimmed.find("__") == std::string::npos &&
           std::none_of(trimmed.begin(), trimmed.end(), [](char c) { return std::isupper(static_cast<unsigned char>(c)) != 0; });
}

// rustc's non_camel_case_types rule: not starting with a lower case letter,
// no double underscore, and no underscore next to a letter.
bool RustGenerator::is_camel_case(const std::string& name)
{
    const std::string trimmed = trim_underscores(name);
    if (trimmed.empty())
    {
        return true;
    }
    if (std::islower(static_cast<unsigned char>(trimmed[0])) || trimmed.find("__") != std::string::npos)
    {
        return false;
    }
    for (size_t i = 0; i + 1 < trimmed.size(); i++)
    {
        if ((has_case(trimmed[i]) && trimmed[i + 1] == '_') || (trimmed[i] == '_' && has_case(trimmed[i + 1])))
        {
            return false;
        }
    }
    return true;
}

std::string RustGenerator::rust_type(const std::string& type)
{
    std::string mapped = lang_spec_.get_value_string(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string RustGenerator::rust_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return lang_spec_.get_value_string("big_endian_str");
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return lang_spec_.get_value_string("little_endian_str");
    return lang_spec_.get_value_string("native_endian_str");
}

// Width in bits of a native numeric type, taken from the trailing digits of
// its Rust name (i8 -> 8, f64 -> 64). Zero for strings and user types.
unsigned int RustGenerator::rust_bits(const std::string& type)
{
    if (is_string_type(type) || !is_native_type(type))
    {
        return 0;
    }

    static const std::regex trailing_digits("([0-9]+)$");
    std::smatch match;
    std::string mapped = rust_type(type);
    if (std::regex_search(mapped, match, trailing_digits))
    {
        return static_cast<unsigned int>(std::stoul(match[1]));
    }
    return 0;
}

bool RustGenerator::is_float_type(const std::string& type)
{
    return type == "float" || type == "double";
}

std::vector<std::string> RustGenerator::accessor_names(const member_desc_t& member)
{
    const std::string& name = std::get<member_name>(member);
    std::vector<std::string> names = { name, "set_" + name };
    if (is_user_type(std::get<member_type>(member)))
    {
        names.push_back(name + "_mut");
    }
    return names;
}

std::string RustGenerator::subst(std::string template_str, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    unsigned int bits = rust_bits(type);
    unsigned int wire_size = is_string_type(type) ? std::get<member_length>(member) : bits / 8;

    template_str = replace_all(template_str, lang_spec_.get_value_string("structname_substr"), classname_);
    template_str = replace_all(template_str, lang_spec_.get_value_string("plain_member_substr"), std::get<member_name>(member));
    template_str = replace_all(template_str, lang_spec_.get_value_string("member_substr"), identifier(std::get<member_name>(member)));
    template_str = replace_all(template_str, lang_spec_.get_value_string("type_substr"), rust_type(type));
    template_str = replace_all(template_str, lang_spec_.get_value_string("endian_substr"), rust_endian(std::get<member_endianness>(member)));
    template_str = replace_all(template_str, lang_spec_.get_value_string("length_substr"), std::to_string(std::get<member_length>(member)));
    template_str = replace_all(template_str, lang_spec_.get_value_string("size_substr"), std::to_string(wire_size));
    // A bitfield setter shifts the field's bits to the top and back, which
    // masks unsigned values and sign-extends signed ones.
    if (is_bitfield(member))
    {
        template_str = replace_all(template_str, lang_spec_.get_value_string("unused_bits_substr"),
            std::to_string(bits - std::get<member_bits>(member)));
    }

    return template_str;
}

bool RustGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    if (false == check_names(errorcode))
    {
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

bool RustGenerator::check_names(int& errorcode)
{
    bool valid = true;

    std::vector<std::string> types = { classname_ };
    for (auto& type : user_types())
    {
        types.push_back(type);
    }
    for (auto& type : types)
    {
        if (unusable_names.count(type) != 0 || rust_keywords.count(type) != 0)
        {
            valid = spec_error(errorcode, "type name '" + type + "' is a Rust keyword");
        }
    }

    std::map<std::string, std::string> taken;  // accessor name -> member that has it
    for (auto& name : generated_names)
    {
        taken[name] = "";
    }

    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        if (unusable_names.count(name) != 0)
        {
            valid = spec_error(errorcode, "member name '" + name + "' can't be a Rust identifier");
            continue;
        }

        for (auto& accessor : accessor_names(member.second))
        {
            auto other = taken.find(accessor);
            if (other == taken.end())
            {
                taken[accessor] = name;
            }
            else if (other->second.empty())
            {
                valid = spec_error(errorcode, "member '" + name + "' needs an accessor " + accessor + "(), which the generated code defines");
            }
            else
            {
                valid = spec_error(errorcode, "members '" + other->second + "' and '" + name + "' both need an accessor " + accessor + "()");
            }
        }
    }

    return valid;
}

bool RustGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_lints(errorcode);
    imp_uses(errorcode);
    imp_struct(errorcode);
    imp_default(errorcode);
    imp_impl(errorcode);

    // rustfmt ends a file with exactly one newline.
    std::string body = ss_.str();
    body.erase(body.find_last_not_of('\n') + 1);
    body += '\n';

    imp_string.append(body);

    return true;
}

bool RustGenerator::imp_banner(int& errorcode)
{
    bool success = gen_one_line("banner_str");

    ss_ << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// Names from the spec are kept as they are, so allow Rust's naming lints
// where the names don't follow them.
bool RustGenerator::imp_lints(int& errorcode)
{
    std::vector<std::string> lints;

    if (false == is_camel_case(classname_))
    {
        lints.push_back("non_camel_case_types");
    }

    for (auto& member : ordered_members_)
    {
        auto names = accessor_names(member.second);
        if (std::any_of(names.begin(), names.end(), [](const std::string& name) { return !is_snake_case(name); }))
        {
            lints.push_back("non_snake_case");
            break;
        }
    }

    if (lints.empty())
    {
        return true;
    }

    std::string joined;
    for (auto& lint : lints)
    {
        joined += (joined.empty() ? "" : ", ") + lint;
    }

    ss_ << replace_all(lang_spec_.get_value_string("allow_str"), lang_spec_.get_value_string("lints_substr"), joined);
    ss_ << lang_spec_.get_value_string("block_separator_str");

    return true;
}

// Nested message types are sibling modules. The spec's includes name C and
// C++ headers or Go packages, so Rust doesn't use them.
bool RustGenerator::imp_uses(int& errorcode)
{
    std::vector<std::string> uses;
    for (auto& type : user_types())
    {
        std::string line = lang_spec_.get_value_string("use_str");
        line = replace_all(line, lang_spec_.get_value_string("module_substr"), module_name(type));
        line = replace_all(line, lang_spec_.get_value_string("type_substr"), type);
        uses.push_back(line);
    }

    if (uses.empty())
    {
        return true;
    }

    // rustfmt sorts use declarations.
    std::sort(uses.begin(), uses.end());
    for (auto& line : uses)
    {
        ss_ << line;
    }
    ss_ << lang_spec_.get_value_string("block_separator_str");

    return true;
}

bool RustGenerator::imp_struct(int& errorcode)
{
    // std implements Default for arrays of up to 32 elements only.
    bool derive_default = true;
    for (auto& member : ordered_members_)
    {
        if (is_string_type(std::get<member_type>(member.second)) && std::get<member_length>(member.second) > 32)
        {
            derive_default = false;
        }
    }

    std::string block = lang_spec_.get_value_string("struct_block");
    block = replace_all(block, lang_spec_.get_value_string("derives_substr"),
        lang_spec_.get_value_string(derive_default ? "derives_str" : "derives_no_default_str"));

    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key = is_string_type(type) ? "member_string_decl_str" : "member_decl_str";

        body += subst(lang_spec_.get_value_string(key), member.second);
    }

    block = replace_all(block, lang_spec_.get_value_string("structname_substr"), classname_);
    block = replace_all(block, lang_spec_.get_value_string("body_token"), body);
    ss_ << block << lang_spec_.get_value_string("block_separator_str");

    return true;
}

// Only written when the struct can't derive Default (a string longer than 32).
bool RustGenerator::imp_default(int& errorcode)
{
    bool needed = false;
    for (auto& member : ordered_members_)
    {
        if (is_string_type(std::get<member_type>(member.second)) && std::get<member_length>(member.second) > 32)
        {
            needed = true;
        }
    }

    if (false == needed)
    {
        return true;
    }

    return do_block("default_block", [this]()->std::string {
        std::string body;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);
            std::string key;

            if (is_string_type(type))
                key = "default_string_mem_str";
            else if (!is_native_type(type))
                key = "default_user_mem_str";
            else if (is_float_type(type))
                key = "default_float_mem_str";
            else
                key = "default_mem_str";

            body += subst(lang_spec_.get_value_string(key), member.second);
        }
        return body;
    });
}

bool RustGenerator::imp_impl(int& errorcode)
{
    return do_block("impl_block", [this]()->std::string {
        const std::string separator = lang_spec_.get_value_string("block_separator_str");
        const std::string body_token = lang_spec_.get_value_string("body_token");

        auto with_body = [&](const std::string& key, const std::string& body) {
            return replace_all(lang_spec_.get_value_string(key), body_token, body);
        };

        std::vector<std::string> items;
        items.push_back(replace_all(lang_spec_.get_value_string("size_const_str"),
            lang_spec_.get_value_string("size_substr"), size_expression()));
        items.push_back(lang_spec_.get_value_string("size_fcn_str"));
        items.push_back(lang_spec_.get_value_string("serialize_fcn_str"));
        items.push_back(lang_spec_.get_value_string("serialize_into_fcn_str"));
        items.push_back(with_body("write_to_fcn_block", serialize_body()));
        items.push_back(with_body("deserialize_fcn_block", deserialize_body()));
        items.push_back(lang_spec_.get_value_string("from_bytes_fcn_str"));
        items.push_back(lang_spec_.get_value_string("equals_fcn_str"));
        for (auto& member : ordered_members_)
        {
            items.push_back(accessors(member.second));
        }

        std::string body;
        for (size_t i = 0; i < items.size(); i++)
        {
            body += (i ? separator : "") + items[i];
        }
        return body;
    });
}

// The fixed bytes as one number, then each nested type's SIZE, counted
// (e.g. "13 + 2 * NestyInner::SIZE"), so the constant stays short.
std::string RustGenerator::size_expression()
{
    unsigned long fixed = 0;
    std::vector<std::string> nested_order;
    std::map<std::string, unsigned int> nested_count;

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);

        if (is_bitfield(member.second))
        {
            // A storage unit counts once, at its first field.
            if (place_of(member.first).first)
                fixed += rust_bits(type) / 8;
        }
        else if (is_string_type(type))
            fixed += std::get<member_length>(member.second);
        else if (!is_native_type(type))
        {
            if (nested_count[type]++ == 0)
                nested_order.push_back(type);
        }
        else
            fixed += rust_bits(type) / 8;
    }

    std::vector<std::string> terms;
    if (fixed != 0 || nested_order.empty())
    {
        terms.push_back(std::to_string(fixed));
    }
    for (auto& type : nested_order)
    {
        unsigned int count = nested_count[type];
        terms.push_back((count > 1 ? std::to_string(count) + " * " : "") + type + "::SIZE");
    }

    std::string expression;
    for (size_t i = 0; i < terms.size(); i++)
    {
        expression += (i ? " + " : "") + terms[i];
    }
    return expression;
}

// The body of write_to: each member written at the front of buf, which then
// moves past it, as deserialize reads. The sizes are constants, so the
// bounds checks fold away.
std::string RustGenerator::serialize_body()
{
    std::string body;
    size_t i = 0;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;
        std::string advance_key = "serialize_advance_str";

        if (is_bitfield(member.second))
        {
            body += serialize_bitfield(member.first, member.second);
            // A unit is written after its last field.
            if (++i < ordered_members_.size() && place_of(member.first).last)
            {
                body += "        let buf = &mut buf[" + std::to_string(rust_bits(type) / 8) + "..];\n";
            }
            continue;
        }

        if (is_string_type(type))
            key = "serialize_string_mem_str";
        else if (!is_native_type(type))
        {
            key = "serialize_user_mem_str";
            advance_key = "serialize_user_advance_str";
        }
        else if (rust_bits(type) == 8)
            key = rust_type(type) == "u8" ? "serialize_byte_mem_str" : "serialize_signed_byte_mem_str";
        else
            key = "serialize_num_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);

        // Nothing is written past the last member, so don't advance after it.
        if (++i < ordered_members_.size())
        {
            body += subst(lang_spec_.get_value_string(advance_key), member.second);
        }
    }
    return body;
}

std::string RustGenerator::deserialize_body()
{
    std::string body;
    size_t i = 0;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;
        std::string advance_key = "deserialize_advance_str";

        if (is_bitfield(member.second))
        {
            body += deserialize_bitfield(member.first, member.second);
            // A unit is consumed after its last field.
            if (++i < ordered_members_.size() && place_of(member.first).last)
            {
                body += "        let buf = &buf[" + std::to_string(rust_bits(type) / 8) + "..];\n";
            }
            continue;
        }

        if (is_string_type(type))
            key = "deserialize_string_mem_str";
        else if (!is_native_type(type))
        {
            key = "deserialize_user_mem_str";
            advance_key = "deserialize_user_advance_str";
        }
        else if (rust_bits(type) == 8)
            key = rust_type(type) == "u8" ? "deserialize_byte_mem_str" : "deserialize_signed_byte_mem_str";
        else
            key = "deserialize_num_mem_str";

        body += subst(lang_spec_.get_value_string(key), member.second);

        // Nothing reads past the last member, so don't advance after it.
        if (++i < ordered_members_.size())
        {
            body += subst(lang_spec_.get_value_string(advance_key), member.second);
        }
    }
    return body;
}

std::string RustGenerator::accessors(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    std::vector<std::string> keys;

    if (is_string_type(type))
        keys = { "get_string_str", "set_string_str" };
    else if (!is_native_type(type))
        keys = { "get_user_str", "mut_user_str", "set_user_str" };
    else if (is_bitfield(member) && std::get<member_bits>(member) < rust_bits(type))
        keys = { "get_str", "set_bits_str" };
    else
        keys = { "get_str", "set_str" };

    std::string text;
    for (size_t i = 0; i < keys.size(); i++)
    {
        text += (i ? lang_spec_.get_value_string("block_separator_str") : "") + subst(lang_spec_.get_value_string(keys[i]), member);
    }
    return text;
}

bool RustGenerator::do_block(const std::string block_id, std::function<std::string()> builder)
{
    bool success = true;
    std::string block_str = lang_spec_.get_value_string(block_id);
    std::string structname_substr = lang_spec_.get_value_string("structname_substr");
    std::string body_token_str = lang_spec_.get_value_string("body_token");

    std::string new_body_str = builder();

    block_str = replace_all(block_str, structname_substr, classname_);
    block_str = replace_all(block_str, body_token_str, new_body_str);

    ss_ << block_str << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// A bitfield's storage unit is a local, u<n>: its first field declares it,
// each further field ors itself in, and the last one writes it. A cast is
// parenthesized only where an operator follows it (`x as u8 << 3` doesn't
// parse), and parentheses go only where they change the meaning, as rustc
// warns about unused ones.
std::string RustGenerator::serialize_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int width = rust_bits(unit.type);
    const std::string utype = "u" + std::to_string(width);
    const std::string var = "u" + std::to_string(place.unit);
    const std::string field = "self." + identifier(std::get<member_name>(member));
    const bool cast = rust_type(std::get<member_type>(member)) != utype;
    const bool masked = place.bits < width;

    auto operand = [&]() { return cast ? "(" + field + " as " + utype + ")" : field; };

    std::string value;
    if (masked)
    {
        value = operand() + " & " + bitfields::hex(bitfields::mask(place.bits));
    }
    if (place.shift)
    {
        value = (masked ? "(" + value + ")" : operand()) + " << " + std::to_string(place.shift);
    }
    if (value.empty())
    {
        value = cast ? field + " as " + utype : field;
    }

    std::string body;
    if (place.first)
    {
        body += std::string("        let ") + (place.last ? "" : "mut ") + var + " = " + value + ";\n";
    }
    else
    {
        body += "        " + var + " |= " + value + ";\n";
    }

    if (place.last)
    {
        body += width == 8 ? "        buf[0] = " + var + ";\n"
                           : "        buf[.." + std::to_string(width / 8) + "].copy_from_slice(&" + var + ".to_" + rust_endian(unit.endian) + "_bytes());\n";
    }

    return body;
}

std::string RustGenerator::deserialize_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int width = rust_bits(unit.type);
    const std::string utype = "u" + std::to_string(width);
    const std::string var = "u" + std::to_string(place.unit);
    const std::string type = rust_type(std::get<member_type>(member));

    std::string body;
    if (place.first)
    {
        body += "        let " + var + " = " +
                (width == 8 ? "buf[0]" : utype + "::from_" + rust_endian(unit.endian) + "_bytes(buf[.." + std::to_string(width / 8) + "].try_into().unwrap())") +
                ";\n";
    }

    // Shifting the field to the top, converting, and shifting it back down
    // masks unsigned fields and sign-extends signed ones.
    const unsigned int up = width - place.shift - place.bits;
    const unsigned int down = width - place.bits;

    std::string value = up ? var + " << " + std::to_string(up) : var;
    if (type != utype)
    {
        value = (up ? "(" + value + ")" : value) + " as " + type;
    }
    if (down)
    {
        value = (up || type != utype ? "(" + value + ")" : value) + " >> " + std::to_string(down);
    }
    body += "        self." + identifier(std::get<member_name>(member)) + " = " + value + ";\n";

    return body;
}

}
