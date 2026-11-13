//
//   Copyright (C) 2019 Geoffrey Atkinson
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
#include <iostream>
#include <regex>
#include <map>
#include <set>
#include <string>
#include "gogenerator.h"
#include "runtime_sources.h"
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

    static const std::regex trailing_digits("([0-9]+)$");
    std::smatch match;
    std::string mapped = go_type(type);
    if (std::regex_search(mapped, match, trailing_digits))
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

    template_str = replace_all(template_str, lang_spec_.get_value_string("structname_substr"), classname_);
    template_str = replace_all(template_str, lang_spec_.get_value_string("member_substr"), std::get<member_name>(member));
    template_str = replace_all(template_str, lang_spec_.get_value_string("type_substr"), go_type(type));
    template_str = replace_all(template_str, lang_spec_.get_value_string("endian_substr"), go_endian(std::get<member_endianness>(member)));
    template_str = replace_all(template_str, lang_spec_.get_value_string("bits_substr"), std::to_string(bits));
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

    if (wants_text())
    {
        if (false == check_text_names(errorcode))
        {
            return false;
        }
        // The JSON, XML and BSON helpers, written into the message's package.
        std::string runtime = runtime_source("go/skywire_text.go");
        const std::string package_line = lang_spec_.get_value_string("runtime_package_str");
        runtime = replace_all(runtime, package_line,
                              "package " + msg_spec_.get_value_string("namespace") + "\n");
        results_["runtime"] = runtime;
    }

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
    imp_text(errorcode);

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
        std::set<std::string> packages = { "fmt", "slices" };

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
        // MarshalXML and UnmarshalXML take encoding/xml's types.
        if (wants_xml())
        {
            packages.insert("encoding/xml");
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
            body += replace_all(import_str, include_substr, package);
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

// Put stores each member at its offset in a, the message as an array
// pointer, so no store needs a bounds check. Offsets past a nested message add
// its Size constant.
bool GoGenerator::imp_serialize(int& errorcode)
{
    return do_block("serialize_fcn_block", [this]()->std::string {
        return members_at("serialize");
    });
}

// Deserialize reads each member at its offset in a, o as an array pointer.
bool GoGenerator::imp_deserialize(int& errorcode)
{
    return do_block("deserialize_fcn_block", [this]()->std::string {
        return members_at("deserialize");
    });
}

// One line per member, reading or writing it at its offset in a.
// direction is "serialize" or "deserialize".
std::string GoGenerator::members_at(const std::string& direction)
{
    const std::string at_substr = lang_spec_.get_value_string("at_substr");
    const std::string index_substr = lang_spec_.get_value_string("index_substr");
    const bool writing = direction == "serialize";
    std::string body;
    unsigned int fixed = 0;
    std::string dynamic;

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        // gofmt writes an index expression without spaces: a[12+RingyInnerSize:].
        const std::string index = (fixed || dynamic.empty() ? std::to_string(fixed) : "") +
                                  (fixed && !dynamic.empty() ? "+" : "") + dynamic;
        const std::string at = "a[" + (index == "0" ? std::string() : index) + ":]";
        std::string key;

        if (is_bitfield(member.second))
        {
            body += writing ? serialize_bitfield(member.first, member.second, index, at)
                            : deserialize_bitfield(member.first, member.second, index, at);
            // A unit is consumed after its last field.
            if (place_of(member.first).last)
            {
                fixed += go_bits(type) / 8;
            }
            continue;
        }

        if (is_string_type(type))
            key = direction + "_string_mem_str";
        else if (!is_native_type(type))
            key = direction + "_user_mem_str";
        else if (go_bits(type) == 8)
            key = direction + "_byte_mem_str";
        else if (is_float_type(type))
            key = direction + "_float_mem_str";
        else
            key = direction + "_int_mem_str";

        std::string line = subst(lang_spec_.get_value_string(key), member.second);
        line = replace_all(line, at_substr, at);
        line = replace_all(line, index_substr, index);
        body += line;

        if (!is_native_type(type))
            dynamic += (dynamic.empty() ? "" : "+") + go_type(type) + "Size";
        else if (is_string_type(type))
            fixed += std::get<member_length>(member.second);
        else
            fixed += go_bits(type) / 8;
    }

    return body;
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

        std::vector<std::string> terms;
        for (auto& member : ordered_members_)
        {
            const std::string& type = std::get<member_type>(member.second);

            if (is_bitfield(member.second))
            {
                // A storage unit counts once, at its first field.
                if (place_of(member.first).first)
                    terms.push_back(std::to_string(go_bits(type) / 8));
            }
            else if (is_string_type(type))
                terms.push_back(std::to_string(std::get<member_length>(member.second)));
            else if (!is_native_type(type))
                terms.push_back(subst(lang_spec_.get_value_string("size_user_mem_str"), member.second));
            else
                terms.push_back(std::to_string(go_bits(type) / 8));
        }

        std::string body;
        for (size_t i = 0; i < terms.size(); i++)
        {
            body += (i ? separator : "") + terms[i];
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
        if (is_bitfield(member.second) && std::get<member_bits>(member.second) < go_bits(type))
        {
            key = "set_bits_str";
        }

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

    block_str = replace_all(block_str, structname_substr, classname_);
    block_str = replace_all(block_str, body_token_str, new_body_str);

    ss_ << block_str << lang_spec_.get_value_string("block_separator_str");

    return success;
}

// A bitfield's storage unit is a local, u<n>: its first field declares it,
// each field ors itself in, and the last one stores it.
std::string GoGenerator::serialize_bitfield(uint32_t tag, const member_desc_t& member, const std::string& index, const std::string& at)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int width = go_bits(unit.type);
    const std::string utype = "uint" + std::to_string(width);
    const std::string var = "u" + std::to_string(place.unit);
    const std::string field = utype + "(s." + std::get<member_name>(member) + ")";

    std::string body;
    if (place.first)
    {
        body += "\t" + var + " := " + utype + "(0)\n";
    }

    std::string value = place.bits < width ? "(" + field + " & " + bitfields::hex(bitfields::mask(place.bits)) + ")" : field;
    body += "\t" + var + " |= " + value + (place.shift ? " << " + std::to_string(place.shift) : "") + "\n";

    if (place.last)
    {
        body += width == 8 ? "\ta[" + index + "] = " + var + "\n"
                           : "\t" + go_endian(unit.endian) + ".PutUint" + std::to_string(width) + "(" + at + ", " + var + ")\n";
    }

    return body;
}

std::string GoGenerator::deserialize_bitfield(uint32_t tag, const member_desc_t& member, const std::string& index, const std::string& at)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int width = go_bits(unit.type);
    const std::string var = "u" + std::to_string(place.unit);

    std::string body;
    if (place.first)
    {
        body += "\t" + var + " := " + (width == 8 ? "a[" + index + "]" : go_endian(unit.endian) + ".Uint" + std::to_string(width) + "(" + at + ")") + "\n";
    }

    // Shifting the field to the top, converting, and shifting it back down
    // masks unsigned fields and sign-extends signed ones.
    const unsigned int up = width - place.shift - place.bits;
    const unsigned int down = width - place.bits;
    std::string value = go_type(std::get<member_type>(member)) + "(" + var + (up ? "<<" + std::to_string(up) : "") + ")";
    if (down)
    {
        value += " >> " + std::to_string(down);
    }
    body += "\ts." + std::get<member_name>(member) + " = " + value + "\n";

    return body;
}

// The methods the text forms add, which a member cannot also be named.
bool GoGenerator::check_text_names(int& errorcode)
{
    std::vector<std::string> reserved;
    if (wants_json())
        reserved.insert(reserved.end(), { "ToJSON", "FromJSON", "MarshalJSON", "UnmarshalJSON", "writeJSON", "readJSON" });
    if (wants_xml())
        reserved.insert(reserved.end(), { "ToXML", "FromXML", "MarshalXML", "UnmarshalXML", "writeXML", "readXML" });
    if (wants_bson())
        reserved.insert(reserved.end(), { "MarshalBSON", "UnmarshalBSON", "writeBSON", "readBSON" });

    bool valid = true;
    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        if (std::find(reserved.begin(), reserved.end(), name) != reserved.end())
        {
            std::cerr << "Member name \"" << name << "\" clashes with the generated method " << name << std::endl;
            errorcode |= SW_SPEC_INVALID;
            valid = false;
        }
    }
    return valid;
}

// One member's lines in a to_ or from_ function. The writer method, and the
// conversion to its argument, is int32 for every integer up to 32 bits but
// uint32_t (a char as signed), otherwise the member's own Go type.
std::string GoGenerator::text_member(const std::string& function, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const bool reading = function.rfind("from_", 0) == 0;
    std::string kind = "native";
    if (is_string_type(type))
        kind = "string";
    else if (!is_native_type(type))
        kind = "user";
    else if (reading)
        kind = is_float_type(type) ? "real" : "int";

    std::string method = go_type(type);
    if (is_native_type(type) && !is_float_type(type) && go_bits(type) <= 32 && type != "uint32_t")
        method = "int32";

    std::string template_str = subst(lang_spec_.get_value_string(function + "_" + kind + "_mem_str"), member);
    template_str = replace_all(template_str, lang_spec_.get_value_string("ctype_substr"), type);
    template_str = replace_all(template_str, lang_spec_.get_value_string("kind_substr"), method);
    return replace_all(template_str, lang_spec_.get_value_string("fieldbits_substr"),
                       std::to_string(is_bitfield(member) ? std::get<member_bits>(member) : 0));
}

// ToJSON/FromJSON (with json.Marshaler and json.Unmarshaler), ToXML/FromXML
// (with xml.Marshaler and xml.Unmarshaler) and MarshalBSON/UnmarshalBSON
// (docs/text-formats.md), each only when asked for, with the write and read
// methods a containing message calls. Members go in tag order; reading
// takes them in any order, leaves missing ones zero, ignores unknown ones
// and on failure changes nothing.
bool GoGenerator::imp_text(int& errorcode)
{
    const std::string structname_substr = lang_spec_.get_value_string("structname_substr");
    std::vector<std::string> formats;
    if (wants_json())
        formats.push_back("json");
    if (wants_xml())
        formats.push_back("xml");
    if (wants_bson())
        formats.push_back("bson");

    for (const auto& format : formats)
    {
        for (const std::string direction : { "to_", "from_" })
        {
            const std::string function = direction + format;
            ss_ << replace_all(lang_spec_.get_value_string(function + "_str"), structname_substr, classname_);
            for (auto& member : ordered_members_)
            {
                ss_ << text_member(function, member.second);
            }
            ss_ << lang_spec_.get_value_string(function + "_end_str");
        }
    }
    return true;
}

}
