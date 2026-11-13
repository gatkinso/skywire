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

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "swiftgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

std::string SwiftGenerator::swift_type(const std::string& type)
{
    std::string mapped = lang_spec_.get_value_string(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string SwiftGenerator::swift_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return ".big";
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return ".little";
    return ".native";
}

bool SwiftGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    ss_.str("");

    if (wants_text() && false == check_text_names(errorcode))
    {
        return false;
    }

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool SwiftGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_imports(errorcode);
    imp_classname(errorcode);
    imp_members(errorcode);
    imp_cons(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);
    ss_ << text_functions();
    imp_classend(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool SwiftGenerator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool SwiftGenerator::imp_imports(int& errorcode)
{
    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");

    if (values.empty() || values[0] == "@NOTFOUND")
    {
        return true;
    }

    const std::string import_str = lang_spec_.get_value_string("import_str");
    const std::string import_substr = lang_spec_.get_value_string("import_substr");

    for (auto& item : values)
    {
        std::string template_str = replace_all(import_str, import_substr, item);
        ss_ << template_str;
    }

    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_classname(int& errorcode)
{
    std::string template_str = lang_spec_.get_value_string("classname_str");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    template_str = replace_all(template_str, classname_substr, classname_);
    ss_ << template_str;

    return true;
}

bool SwiftGenerator::imp_members(int& errorcode)
{
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("member_string_decl_str");
            template_str = replace_all(template_str, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string(is_user_type(std::get<member_type>(member.second)) ? "member_user_decl_str" : "member_decl_str");
            template_str = replace_all(template_str, type_substr, swift_type(std::get<member_type>(member.second)));
        }

        template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        ss_ << template_str;
    }

    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_cons(int& errorcode)
{
    insert_indents(1);
    gen_one_line("cons_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_serialize(int& errorcode)
{
    insert_indents(1);
    gen_one_line("serialize_str");

    // Claim the whole message from the stream at once, then write it in
    // place with serialize(into:), which a containing message also calls.
    gen_lines(lang_spec_.get_value_string("serialize_claim_str"), 2);
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;
    gen_lines(lang_spec_.get_value_string("serialize_into_str"), 1);

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_bitfield(member.second))
        {
            serialize_bitfield(member.first, member.second);
            continue;
        }

        insert_indents(2);

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("serialize_string_mem_str");
            template_str = replace_all(template_str, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("serialize_native_mem_str");
            template_str = replace_all(template_str, endian_substr, swift_endian(std::get<member_endianness>(member.second)));
        }
        else
        {
            // A nested message uses its own members' byte orders.
            template_str = lang_spec_.get_value_string("serialize_user_mem_str");
        }

        template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        ss_ << template_str << std::endl;
    }

    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_deserialize(int& errorcode)
{
    insert_indents(1);
    gen_one_line("deserialize_str");

    // Take the whole message from the stream at once, rejecting a short
    // stream before touching any field, then read it with deserialize(from:).
    gen_lines(lang_spec_.get_value_string("deserialize_consume_str"), 2);
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;
    gen_lines(lang_spec_.get_value_string("deserialize_from_str"), 1);

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_bitfield(member.second))
        {
            deserialize_bitfield(member.first, member.second);
            continue;
        }

        insert_indents(2);

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_string_mem_str");
            template_str = replace_all(template_str, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("deserialize_native_mem_str");
            template_str = replace_all(template_str, endian_substr, swift_endian(std::get<member_endianness>(member.second)));
        }
        else
        {
            // A nested message uses its own members' byte orders.
            template_str = lang_spec_.get_value_string("deserialize_user_mem_str");
        }

        template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        ss_ << template_str << std::endl;
    }

    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_size(int& errorcode)
{
    insert_indents(1);
    gen_one_line("size_str");

    insert_indents(2);
    ss_ << "return (" << std::endl;

    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    std::vector<std::string> terms;
    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_bitfield(member.second))
        {
            // A storage unit counts once, at its first field.
            if (place_of(member.first).first)
                terms.push_back("MemoryLayout<" + swift_type(bitfields::unsigned_type(unit_of(member.first).type)) + ">.size");
            continue;
        }

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_string_mem_str");
            template_str = replace_all(template_str, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_native_mem_str");
            template_str = replace_all(template_str, type_substr, swift_type(std::get<member_type>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("size_user_mem_str");
            template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        }

        terms.push_back(template_str);
    }

    for (size_t i = 0; i < terms.size(); i++)
    {
        insert_indents(3);
        ss_ << terms[i] << (i + 1 < terms.size() ? " +" : "") << std::endl;
    }

    insert_indents(2);
    ss_ << ")" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_equals(int& errorcode)
{
    std::string template_str = lang_spec_.get_value_string("equals_str");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    template_str = replace_all(template_str, classname_substr, classname_);
    insert_indents(1);
    ss_ << template_str;

    insert_indents(2);
    ss_ << "return (" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string member_tmpl;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            member_tmpl = lang_spec_.get_value_string("equals_string_mem_str");
            member_tmpl = replace_all(member_tmpl, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_user_type(std::get<member_type>(member.second)))
        {
            member_tmpl = lang_spec_.get_value_string("equals_user_mem_str");
        }
        else
        {
            member_tmpl = lang_spec_.get_value_string("equals_mem_str");
        }

        member_tmpl = replace_all(member_tmpl, member_substr, std::get<member_name>(member.second));
        insert_indents(3);
        ss_ << member_tmpl;

        if (i++ < static_cast<int>(ordered_members_.size()) - 1)
        {
            ss_ << " &&";
        }

        ss_ << std::endl;
    }

    insert_indents(2);
    ss_ << ")" << std::endl;
    insert_indents(1);
    gen_one_line("endfunction_str");
    ss_ << std::endl;

    return true;
}

bool SwiftGenerator::imp_get_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("get_string_str");
        }
        else
        {
            template_str = lang_spec_.get_value_string("get_str");
            template_str = replace_all(template_str, type_substr, swift_type(std::get<member_type>(member.second)));
        }

        template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool SwiftGenerator::imp_set_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("set_string_str");
            template_str = replace_all(template_str, length_substr, std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_bitfield(member.second) && std::get<member_bits>(member.second) < bitfields::width(std::get<member_type>(member.second)))
        {
            // Shifting the field's bits to the top and back masks unsigned
            // values and sign-extends signed ones.
            const std::string unused = std::to_string(bitfields::width(std::get<member_type>(member.second)) - std::get<member_bits>(member.second));
            template_str = "public func Set___MEMBERNAME__(_ newval: " + swift_type(std::get<member_type>(member.second)) +
                           ") { __MEMBERNAME__ = (newval << " + unused + ") >> " + unused + " }\n";
        }
        else
        {
            template_str = lang_spec_.get_value_string("set_str");
            template_str = replace_all(template_str, type_substr, swift_type(std::get<member_type>(member.second)));
        }

        template_str = replace_all(template_str, member_substr, std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool SwiftGenerator::imp_classend(int& errorcode)
{
    return gen_one_line("endclass_str");
}

// A bitfield's storage unit is a local, unit_<n>: its first field declares
// it, each field ors itself in, and the last one writes it.
void SwiftGenerator::serialize_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const std::string utype = swift_type(bitfields::unsigned_type(unit.type));
    const std::string var = "unit_" + std::to_string(place.unit);

    if (place.first)
    {
        insert_indents(2);
        ss_ << "var " << var << ": " << utype << " = 0" << std::endl;
    }

    std::string field = utype + "(truncatingIfNeeded: " + std::get<member_name>(member) + ")";
    if (place.bits < bitfields::width(unit.type))
    {
        field = "(" + field + " & " + bitfields::hex(bitfields::mask(place.bits)) + ")";
    }
    insert_indents(2);
    ss_ << var << " |= " << field << (place.shift ? " << " + std::to_string(place.shift) : "") << std::endl;

    if (place.last)
    {
        insert_indents(2);
        ss_ << "SWWire.put(&p, " << var << ", " << swift_endian(unit.endian) << ")" << std::endl;
    }
}

void SwiftGenerator::deserialize_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int width = bitfields::width(unit.type);
    const std::string var = "unit_" + std::to_string(place.unit);

    if (place.first)
    {
        insert_indents(2);
        ss_ << "let " << var << ": " << swift_type(bitfields::unsigned_type(unit.type)) << " = SWWire.get(&p, "
            << swift_endian(unit.endian) << ")" << std::endl;
    }

    // Shifting the field to the top, converting, and shifting it back down
    // masks unsigned fields and sign-extends signed ones.
    const unsigned int up = width - place.shift - place.bits;
    const unsigned int down = width - place.bits;
    std::string value = swift_type(std::get<member_type>(member)) + "(truncatingIfNeeded: " +
                        (up ? var + " << " + std::to_string(up) : var) + ")";
    if (down)
    {
        value += " >> " + std::to_string(down);
    }
    insert_indents(2);
    ss_ << std::get<member_name>(member) << " = " << value << std::endl;
}

// A member is a property, which cannot share a name with a method, and the
// class refers to the helpers by the name SkywireText, which a class or
// member of that name would hide.
bool SwiftGenerator::check_text_names(int& errorcode)
{
    const std::string runtime_type = lang_spec_.get_value_string("runtime_type_str");
    std::set<std::string> reserved = { runtime_type };
    const std::vector<std::pair<bool, std::string>> formats = {
        { wants_json(), "Json" }, { wants_xml(), "Xml" }, { wants_bson(), "Bson" },
    };
    for (auto& format : formats)
    {
        if (format.first)
        {
            reserved.insert({ "to" + format.second, "write" + format.second, "read" + format.second });
        }
    }

    bool valid = true;
    if (classname_ == runtime_type)
    {
        std::cerr << "Invalid specification for Swift: with --generate_json, --generate_xml or --generate_bson a message "
                     "cannot be named "
                  << runtime_type << std::endl;
        valid = false;
    }
    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        if (reserved.count(name) != 0)
        {
            std::cerr << "Invalid specification for Swift: member name '" << name
                      << "' is taken by --generate_json, --generate_xml or --generate_bson" << std::endl;
            valid = false;
        }
    }
    if (false == valid)
    {
        errorcode |= SW_SPEC_INVALID;
    }
    return valid;
}

// One member's lines in a write or read method. The writer method is u32,
// i64, u64, f32 or f64 for those types and i32 for every other integer (a
// char as signed), the value converted to Int32 unless it is one; a read
// integer is truncated to the member's type (it is in range), a read real
// converted.
std::string SwiftGenerator::text_member(const std::string& function, const member_desc_t& member)
{
    static const std::map<std::string, std::string> kinds = {
        { "uint32_t", "u32" }, { "int64_t", "i64" }, { "uint64_t", "u64" }, { "float", "f32" }, { "double", "f64" },
    };

    const std::string& type = std::get<member_type>(member);
    const std::string& name = std::get<member_name>(member);
    const bool reading = function.rfind("from_", 0) == 0;
    std::string kind = "native";
    if (is_string_type(type))
        kind = "string";
    else if (!is_native_type(type))
        kind = "user";
    else if (reading)
        kind = type == "float" || type == "double" ? "real" : "int";

    std::string method = "i32";
    std::string value = "self." + name;
    auto it = kinds.find(type);
    if (it != kinds.end())
        method = it->second;
    else if (swift_type(type) != "Int32")
        value = "Int32(" + value + ")";

    std::string template_str = lang_spec_.get_value_string(function + "_" + kind + "_mem_str");
    template_str = replace_all(template_str, lang_spec_.get_value_string("value_substr"), value);
    template_str = replace_all(template_str, lang_spec_.get_value_string("ctype_substr"), type);
    template_str = replace_all(template_str, lang_spec_.get_value_string("kind_substr"), method);
    template_str = replace_all(template_str, lang_spec_.get_value_string("fieldbits_substr"),
                               std::to_string(is_bitfield(member) ? std::get<member_bits>(member) : 0));
    template_str = replace_all(template_str, lang_spec_.get_value_string("type_substr"), swift_type(type));
    return replace_all(template_str, lang_spec_.get_value_string("member_substr"), name);
}

// toJson/init(json:), toXml/init(xml:) and toBson/init(bson:)
// (docs/text-formats.md), each only when asked for, with the write and read
// methods a containing message calls. Members go in tag order; reading
// takes them in any order, leaves missing ones as they are and ignores
// unknown ones.
std::string SwiftGenerator::text_functions()
{
    const std::string body_token = "__BLOCK_BODY__";
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    auto value = [&](const std::string& key) {
        return replace_all(lang_spec_.get_value_string(key), classname_substr, classname_);
    };
    auto with_body = [&](const std::string& key, const std::string& body) {
        return replace_all(value(key), body_token, body);
    };

    std::vector<std::string> formats;
    if (wants_json())
        formats.push_back("json");
    if (wants_xml())
        formats.push_back("xml");
    if (wants_bson())
        formats.push_back("bson");

    std::string body;
    for (const auto& format : formats)
    {
        std::string writes;
        std::string cases;
        for (auto& member : ordered_members_)
        {
            writes += text_member("to_" + format, member.second);
            cases += replace_all(with_body("read_case_block", text_member("from_" + format, member.second)),
                                 lang_spec_.get_value_string("member_substr"), std::get<member_name>(member.second));
        }
        body += "\n" + value("to_" + format + "_fcn_str");
        body += "\n" + value("from_" + format + "_fcn_str");
        body += "\n" + with_body("write_" + format + "_fcn_block", writes);
        body += "\n" + with_body("read_" + format + "_fcn_block", with_body("read_switch_block", cases));
    }
    return body;
}

}
