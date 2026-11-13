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
#include <iostream>
#include <map>
#include <set>
#include <string>
#include "csharpgenerator.h"
#include "runtime_sources.h"
#include "sw_endian.h"

namespace skyloupe
{

namespace
{

// C#'s reserved keywords; contextual ones (var, value, async, ...) are
// usable as member names.
const std::set<std::string> csharp_keywords = {
    "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked", "class", "const",
    "continue", "decimal", "default", "delegate", "do", "double", "else", "enum", "event", "explicit", "extern",
    "false", "finally", "fixed", "float", "for", "foreach", "goto", "if", "implicit", "in", "int", "interface",
    "internal", "is", "lock", "long", "namespace", "new", "null", "object", "operator", "out", "override",
    "params", "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short",
    "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true", "try", "typeof",
    "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual", "void", "volatile", "while"
};

// What the generated class itself defines or inherits: no member may take
// one of these names.
const std::set<std::string> generated_names = {
    "Size", "Serialize", "Deserialize", "Equals", "GetHashCode", "ToString", "GetType", "MemberwiseClone",
    "ReferenceEquals", "Finalize", "Require", "FloatBits", "BytesEqual", "BytesHash", "PutBytes", "GetBytes",
    "PutLE16", "GetLE16", "PutBE16", "GetBE16", "PutNE16", "GetNE16",
    "PutLE32", "GetLE32", "PutBE32", "GetBE32", "PutNE32", "GetNE32",
    "PutLE64", "GetLE64", "PutBE64", "GetBE64", "PutNE64", "GetNE64"
};

// What the text forms add: the methods of each, and the runtime's types,
// which a member's property would hide.
const std::vector<std::string> json_names = { "ToJson", "FromJson", "WriteJson", "ReadJson" };
const std::vector<std::string> xml_names = { "ToXml", "FromXml", "WriteXml", "ReadXml" };
const std::vector<std::string> bson_names = { "ToBson", "FromBson", "WriteBson", "ReadBson" };
const std::vector<std::string> runtime_names = {
    "SwText", "SwInvalidException", "SwJson", "SwJsonMember", "SwJsonWriter",
    "SwXml", "SwXmlWriter", "SwBson", "SwBsonElement", "SwBsonWriter"
};

bool spec_error(int& errorcode, const std::string& what)
{
    errorcode |= SW_SPEC_INVALID;
    std::cerr << "Invalid specification for C#: " << what << std::endl;
    return false;
}

// The unsigned type a native member of the given wire size goes through.
std::string wire_type(unsigned int bytes)
{
    switch (bytes)
    {
        case 2:
            return "ushort";
        case 4:
            return "uint";
        case 8:
            return "ulong";
        default:
            return "byte";
    }
}

bool is_signed_cs(const std::string& cstype)
{
    return cstype == "sbyte" || cstype == "short" || cstype == "int" || cstype == "long";
}

std::string indent_lines(const std::string& text, const std::string& indent)
{
    std::string out;
    size_t start = 0;
    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        end = end == std::string::npos ? text.size() : end + 1;
        // Preprocessor directives stay in the first column.
        if (text[start] != '\n' && text[start] != '#')
        {
            out += indent;
        }
        out.append(text, start, end - start);
        start = end;
    }
    return out;
}

}

unsigned int CSharpGenerator::wire_bytes(const std::string& type)
{
    static const std::map<std::string, unsigned int> bytes = {
        { "char", 1 }, { "unsigned char", 1 }, { "uint8_t", 1 }, { "int8_t", 1 },
        { "uint16_t", 2 }, { "int16_t", 2 },
        { "uint32_t", 4 }, { "int32_t", 4 }, { "float", 4 },
        { "uint64_t", 8 }, { "int64_t", 8 }, { "double", 8 }
    };

    auto it = bytes.find(type);
    return it == bytes.end() ? 0 : it->second;
}

std::string CSharpGenerator::identifier(const std::string& name)
{
    return csharp_keywords.count(name) != 0 ? "@" + name : name;
}

std::string CSharpGenerator::field_name(const std::string& name)
{
    return "m_" + name;
}

std::string CSharpGenerator::value(const std::string& key)
{
    return lang_spec_.get_value_string(key);
}

std::string CSharpGenerator::cs_type(const std::string& type)
{
    std::string mapped = value(type);
    if (mapped == "@NOTFOUND")
    {
        return type;
    }
    return mapped;
}

std::string CSharpGenerator::cs_endian(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return value("big_endian_str");
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return value("little_endian_str");
    return value("native_endian_str");
}

std::string CSharpGenerator::replace(std::string template_str, const std::string& substr_key, const std::string& with)
{
    return replace_all(template_str, value(substr_key), with);
}

std::string CSharpGenerator::subst(std::string template_str, const member_desc_t& member)
{
    const std::string& name = std::get<member_name>(member);

    template_str = replace(template_str, "classname_substr", classname_);
    template_str = replace(template_str, "member_substr", identifier(name));
    template_str = replace(template_str, "name_substr", name);
    template_str = replace(template_str, "field_substr", field_name(name));
    template_str = replace(template_str, "type_substr", cs_type(std::get<member_type>(member)));
    template_str = replace(template_str, "endian_substr", cs_endian(std::get<member_endianness>(member)));
    template_str = replace(template_str, "size_substr", std::to_string(wire_bytes(std::get<member_type>(member))));
    template_str = replace(template_str, "bits_substr", std::to_string(std::get<member_bits>(member)));
    template_str = replace(template_str, "length_substr", std::to_string(std::get<member_length>(member)));

    return template_str;
}

std::string CSharpGenerator::block(const std::string& block_id, const std::string& body)
{
    std::string block_str = value(block_id);
    block_str = replace(block_str, "classname_substr", classname_);
    block_str = replace(block_str, "body_token", body);
    return block_str;
}

// A native member's value as the bits that go on the wire: a byte, or the
// ushort, uint or ulong its Put helper takes.
std::string CSharpGenerator::to_wire(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string cstype = cs_type(type);
    std::string key;

    if (type == "float")
        key = "to_wire_float_str";
    else if (type == "double")
        key = "to_wire_double_str";
    else if (cstype == "byte" || cstype == "sbyte")
        key = "to_wire_" + cstype + "_str";
    else
        key = is_signed_cs(cstype) ? "to_wire_signed_str" : "to_wire_unsigned_str";

    return subst(replace(value(key), "unsigned_type_substr", wire_type(wire_bytes(type))), member);
}

// A native member's value from wire, the expression reading its bits.
std::string CSharpGenerator::from_wire(const member_desc_t& member, const std::string& wire)
{
    const std::string& type = std::get<member_type>(member);
    std::string key;

    if (type == "float")
        key = "from_wire_float_str";
    else if (type == "double")
        key = "from_wire_double_str";
    else if (cs_type(type) == wire_type(wire_bytes(type)))
        key = "from_wire_plain_str";
    else
        key = "from_wire_integer_str";

    return subst(replace(value(key), "wire_substr", wire), member);
}

std::string CSharpGenerator::read(unsigned int bytes, unsigned int endian, const std::string& at)
{
    if (bytes == 1)
    {
        return replace(value("read_byte_str"), "offset_substr", at);
    }
    return replace(helper(value("read_str"), bytes, endian), "offset_substr", at);
}

// Names a Put/Get helper in a template: its byte order and width in bits.
std::string CSharpGenerator::helper(std::string template_str, unsigned int bytes, unsigned int endian)
{
    template_str = replace(template_str, "endian_substr", cs_endian(endian));
    return replace(template_str, "width_substr", std::to_string(bytes * 8));
}

// The fixed-size members between offset and the next nested message are
// read and written at constant displacements from a local copy of offset,
// o; offset is stored back before a nested message and at the end.
std::string CSharpGenerator::Cursor::at(unsigned int bytes)
{
    std::string code;
    if (false == declared)
    {
        code = gen.value("offset_decl_str");
        declared = loaded = true;
    }
    else if (false == loaded)
    {
        code = gen.value("offset_load_str");
        loaded = true;
    }
    const std::string local = gen.value("offset_local_str");
    index = displacement == 0 ? local : local + " + " + std::to_string(displacement);
    displacement += bytes;
    return code;
}

std::string CSharpGenerator::Cursor::store()
{
    std::string code;
    if (loaded && displacement != 0)
    {
        const std::string local = gen.value("offset_local_str");
        code = gen.replace(gen.value("offset_store_str"), "offset_substr", local + " + " + std::to_string(displacement));
    }
    loaded = false;
    displacement = 0;
    return code;
}

bool CSharpGenerator::generate(int& errorcode)
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

    if (wants_text())
    {
        // The JSON, XML and BSON helpers, written beside the class.
        results_["runtime"] = runtime_source("csharp/SkywireText.cs");
    }

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

// Every name the class declares must be unique within it: the members'
// properties (or a string's Get_/Set_ methods) and fields, and what the
// generated code defines.
bool CSharpGenerator::check_names(int& errorcode)
{
    bool valid = true;

    std::vector<std::string> types = { classname_ };
    for (auto& type : user_types())
    {
        types.push_back(type);
    }
    for (auto& type : types)
    {
        if (csharp_keywords.count(type) != 0)
        {
            valid = spec_error(errorcode, "type name '" + type + "' is a C# keyword");
        }
    }

    std::map<std::string, std::string> taken; // declared name -> member that has it
    for (auto& name : generated_names)
    {
        taken[name] = "";
    }
    std::vector<std::vector<std::string>> text_names;
    if (wants_json())
        text_names.push_back(json_names);
    if (wants_xml())
        text_names.push_back(xml_names);
    if (wants_bson())
        text_names.push_back(bson_names);
    if (wants_text())
        text_names.push_back(runtime_names);
    for (auto& names : text_names)
    {
        for (auto& name : names)
        {
            taken[name] = "";
        }
    }
    taken[classname_] = ""; // a member may not be named after its class

    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);

        std::vector<std::string> declared = { field_name(name) };
        if (is_string_type(std::get<member_type>(member.second)))
        {
            declared.push_back("Get_" + name);
            declared.push_back("Set_" + name);
        }
        else
        {
            declared.push_back(name);
        }

        for (auto& declaration : declared)
        {
            auto other = taken.find(declaration);
            if (other == taken.end())
            {
                taken[declaration] = name;
            }
            else if (other->second.empty())
            {
                valid = spec_error(errorcode, "member '" + name + "' needs the name " + declaration + ", which the generated class uses");
            }
            else
            {
                valid = spec_error(errorcode, "members '" + other->second + "' and '" + name + "' both need the name " + declaration);
            }
        }
    }

    return valid;
}

bool CSharpGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    std::string body = imp_class();

    const std::string ns = msg_spec_.get_value_string("namespace", "@NOTFOUND");
    if (false == ns.empty() && ns != "@NOTFOUND")
    {
        body = replace(block("namespace_block", indent_lines(body, value("indent_str"))), "namespace_substr", ns);
    }

    const std::string usings = imp_usings();
    ss_ << value("banner_str");
    if (uses_inlined_helpers())
    {
        ss_ << value("span_define_str");
    }
    ss_ << value("block_separator_str");
    ss_ << usings << value("block_separator_str");
    ss_ << body;

    imp_string.append(ss_.str());

    return true;
}

// Whether the class has Put/Get or PutBytes/GetBytes helpers: they are
// AggressiveInlining, with a Span fast path where the target has it.
bool CSharpGenerator::uses_inlined_helpers()
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        if (is_string_type(type) || wire_bytes(is_bitfield(member.second) ? unit_of(member.first).type : type) > 1)
        {
            return true;
        }
    }
    return false;
}

// What the generated code uses, plus any extra namespaces the spec lists
// (e.g. for user-defined member types in another namespace).
std::string CSharpGenerator::imp_usings()
{
    std::set<std::string> usings = { "System" };

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        if (wire_bytes(type) != 0)
        {
            usings.insert("System.Globalization");
        }
        if (type == "float")
        {
            usings.insert("System.Runtime.InteropServices");
        }
    }

    if (uses_inlined_helpers())
    {
        usings.insert("System.Runtime.CompilerServices");
    }
    if (wants_text())
    {
        usings.insert(value("runtime_using_str"));
    }

    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
    for (auto& item : values)
    {
        if (item != "@NOTFOUND")
        {
            usings.insert(item);
        }
    }

    std::string out;
    for (auto& name : usings)
    {
        out += replace(value("using_str"), "include_substr", name);
    }
    return out;
}

std::string CSharpGenerator::imp_class()
{
    const std::string separator = value("block_separator_str");

    std::string body = imp_fields();
    body += separator + imp_properties();
    body += separator + block("size_fcn_block", imp_size());
    body += separator + block("serialize_array_fcn_block", "");
    body += separator + block("serialize_fcn_block", imp_serialize());
    body += separator + block("deserialize_array_fcn_block", "");
    body += separator + block("deserialize_fcn_block", imp_deserialize());
    body += separator + block("equals_fcn_block", imp_equals());
    body += separator + block("hashcode_fcn_block", imp_hashcode());
    body += separator + block("tostring_fcn_block", imp_tostring());
    body += imp_text();
    body += imp_helpers();

    return block("class_block", body);
}

std::string CSharpGenerator::imp_fields()
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

        body += subst(value(key), member.second);
    }

    return body;
}

// One-line properties sit together; an accessor with a doc comment or a body
// gets a blank line on each side.
std::string CSharpGenerator::imp_properties()
{
    const std::string separator = value("block_separator_str");

    std::string body;
    bool last_multiline = false;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string accessor;

        if (is_string_type(type))
            accessor = subst(value("get_string_str"), member.second) + separator + subst(value("set_string_str"), member.second);
        else if (!is_native_type(type))
            accessor = subst(value("property_user_str"), member.second);
        else if (is_bitfield(member.second))
            accessor = set_bitfield(member.second);
        else
            accessor = subst(value("property_str"), member.second);

        const bool multiline = std::count(accessor.begin(), accessor.end(), '\n') > 1;
        if (false == body.empty() && (multiline || last_multiline))
        {
            body += separator;
        }
        body += accessor;
        last_multiline = multiline;
    }

    return body;
}

std::string CSharpGenerator::imp_size()
{
    const std::string separator = value("size_separator_str");

    unsigned int fixed = 0;
    std::vector<std::string> terms;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);

        if (is_bitfield(member.second))
        {
            // A storage unit counts once, at its first field.
            if (place_of(member.first).first)
                fixed += wire_bytes(type);
        }
        else if (is_string_type(type))
            fixed += std::get<member_length>(member.second);
        else if (!is_native_type(type))
            terms.push_back(subst(value("size_user_mem_str"), member.second));
        else
            fixed += wire_bytes(type);
    }

    std::string body = std::to_string(fixed);
    if (fixed == 0 && false == terms.empty())
    {
        body.clear();
    }
    for (auto& term : terms)
    {
        body += (body.empty() ? "" : separator) + term;
    }

    return body;
}

std::string CSharpGenerator::imp_serialize()
{
    Cursor cursor(*this);
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);

        if (is_bitfield(member.second))
        {
            body += serialize_bitfield(member.first, member.second, cursor);
        }
        else if (is_string_type(type))
        {
            body += cursor.at(std::get<member_length>(member.second));
            body += subst(replace(value("serialize_string_mem_str"), "offset_substr", cursor.index), member.second);
        }
        else if (!is_native_type(type))
        {
            body += cursor.store();
            body += subst(value("serialize_user_mem_str"), member.second);
        }
        else
        {
            const unsigned int bytes = wire_bytes(type);
            std::string code = bytes == 1 ? value("serialize_byte_mem_str") : helper(value("serialize_mem_str"), bytes, std::get<member_endianness>(member.second));
            body += cursor.at(bytes);
            code = replace(code, "offset_substr", cursor.index);
            body += subst(replace(code, "wire_substr", to_wire(member.second)), member.second);
        }
    }
    body += cursor.store();

    return body;
}

std::string CSharpGenerator::imp_deserialize()
{
    Cursor cursor(*this);
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);

        if (is_bitfield(member.second))
        {
            body += deserialize_bitfield(member.first, member.second, cursor);
        }
        else if (is_string_type(type))
        {
            body += cursor.at(std::get<member_length>(member.second));
            body += subst(replace(value("deserialize_string_mem_str"), "offset_substr", cursor.index), member.second);
        }
        else if (!is_native_type(type))
        {
            body += cursor.store();
            body += subst(value("deserialize_user_mem_str"), member.second);
        }
        else
        {
            const unsigned int bytes = wire_bytes(type);
            body += cursor.at(bytes);
            const std::string wire = from_wire(member.second, read(bytes, std::get<member_endianness>(member.second), cursor.index));
            body += subst(replace(value("deserialize_mem_str"), "wire_substr", wire), member.second);
        }
    }
    body += cursor.store();

    return body;
}

// Floating-point members compare and hash by their bits, so Equals agrees
// with GetHashCode on every runtime (NaN equals itself, -0.0 differs from
// 0.0) and equal messages serialize identically.
std::string CSharpGenerator::imp_equals()
{
    const std::string separator = value("equals_separator_str");

    std::string body;
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

        body += (body.empty() ? "" : separator) + subst(value(key), member.second);
    }

    return body;
}

std::string CSharpGenerator::imp_hashcode()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "hashcode_string_mem_str";
        else if (type == "float")
            key = "hashcode_float_mem_str";
        else if (type == "double")
            key = "hashcode_double_mem_str";
        else
            key = "hashcode_mem_str";

        body += subst(value(key), member.second);
    }

    return body;
}

std::string CSharpGenerator::imp_tostring()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "tostring_string_mem_str";
        else if (!is_native_type(type))
            key = "tostring_user_mem_str";
        else
            key = "tostring_mem_str";

        body += subst(replace(value(key), "separator_substr", body.empty() ? "" : ", "), member.second);
    }

    return body;
}

// The private helpers, only those this class uses.
std::string CSharpGenerator::imp_helpers()
{
    const std::string separator = value("block_separator_str");

    // The Put/Get helpers by width and byte order: a native-order one calls
    // both others, and a 64-bit one the 32-bit one of its order.
    std::set<std::pair<unsigned int, std::string>> helpers;
    bool floats = false, strings = false;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const unsigned int bytes = wire_bytes(is_bitfield(member.second) ? unit_of(member.first).type : type);
        const unsigned int endian = is_bitfield(member.second) ? unit_of(member.first).endian : std::get<member_endianness>(member.second);

        if (bytes > 1)
        {
            const std::string order = cs_endian(endian);
            helpers.insert({ bytes, order });
            if (order == value("native_endian_str"))
            {
                helpers.insert({ bytes, value("little_endian_str") });
                helpers.insert({ bytes, value("big_endian_str") });
            }
        }
        floats = floats || type == "float";
        strings = strings || is_string_type(type);
    }
    for (auto& order : { value("little_endian_str"), value("big_endian_str") })
    {
        if (helpers.count({ 8, order }) != 0)
            helpers.insert({ 4, order });
    }

    std::string body = separator + block("require_fcn_block", "");
    for (unsigned int bytes : { 2, 4, 8 })
    {
        for (auto& order : { value("little_endian_str"), value("big_endian_str") })
        {
            if (helpers.count({ bytes, order }) != 0)
            {
                std::string key = order + std::to_string(bytes * 8) + "_fcn_block";
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                body += separator + block(key, "");
            }
        }
        if (helpers.count({ bytes, value("native_endian_str") }) != 0)
        {
            const std::string native = replace(block("ne_fcn_block", ""), "unsigned_type_substr", wire_type(bytes));
            body += separator + replace(native, "width_substr", std::to_string(bytes * 8));
        }
    }
    if (strings)
        body += separator + block("copy_bytes_fcn_block", "") + separator + block("bytes_fcn_block", "");
    if (floats)
        body += separator + block("float_bits_block", "");

    return body;
}

// One member's lines in a to_ or from_ method. The writer method is Int32
// for every integer up to 32 bits but uint32_t (a char as signed), UInt32,
// Int64, UInt64, Float or Double otherwise.
std::string CSharpGenerator::text_member(const std::string& function, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const bool reading = function.rfind("from_", 0) == 0;
    std::string kind = "native";
    if (is_string_type(type))
        kind = "string";
    else if (!is_native_type(type))
        kind = "user";
    else if (reading)
        kind = type == "float" || type == "double" ? "real" : "int";

    std::string method;
    if (type == "float")
        method = "Float";
    else if (type == "double")
        method = "Double";
    else if (type == "uint32_t")
        method = "UInt32";
    else if (type == "int64_t")
        method = "Int64";
    else if (type == "uint64_t")
        method = "UInt64";
    else
        method = "Int32";

    std::string template_str = value(function + "_" + kind + "_mem_str");
    template_str = replace(template_str, "ctype_substr", type);
    template_str = replace(template_str, "kind_substr", method);
    template_str = replace(template_str, "fieldbits_substr",
                           std::to_string(is_bitfield(member) ? std::get<member_bits>(member) : 0));
    return subst(template_str, member);
}

// ToJson/FromJson, ToXml/FromXml and ToBson/FromBson (docs/text-formats.md),
// each only when asked for, with the write and read methods a containing
// message calls. Members go in tag order; reading takes them in any order,
// leaves missing ones zero, ignores unknown ones and on failure changes
// nothing.
std::string CSharpGenerator::imp_text()
{
    const std::string separator = value("block_separator_str");
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
        for (const std::string direction : { "to_", "from_" })
        {
            const std::string function = direction + format;
            body += separator + block(function + "_str", "");
            for (auto& member : ordered_members_)
            {
                body += text_member(function, member.second);
            }
            body += value(function + "_end_str");
        }
    }
    return body;
}

// A bitfield's storage unit is a ulong local, unit_<n>: its first field
// declares it, each field ors itself in, and the last one writes it.
std::string CSharpGenerator::serialize_bitfield(uint32_t tag, const member_desc_t& member, Cursor& cursor)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int bytes = wire_bytes(unit.type);
    const std::string var = "unit_" + std::to_string(place.unit);
    const std::string field = field_name(std::get<member_name>(member));

    std::string body;
    if (place.first)
    {
        body += "        ulong " + var + " = 0;\n";
    }

    std::string bits = is_signed_cs(cs_type(std::get<member_type>(member))) ? "unchecked((ulong)" + field + ")" : field;
    if (place.bits < 64)
    {
        bits = "(" + bits + " & " + bitfields::hex(bitfields::mask(place.bits)) + "UL)";
    }
    body += "        " + var + " |= " + bits + (place.shift ? " << " + std::to_string(place.shift) : "") + ";\n";

    if (place.last)
    {
        body += cursor.at(bytes);
        const std::string wire = bytes == 8 ? var : "unchecked((" + wire_type(bytes) + ")" + var + ")";
        const std::string code = bytes == 1 ? value("serialize_byte_mem_str") : helper(value("serialize_mem_str"), bytes, unit.endian);
        body += replace(replace(code, "offset_substr", cursor.index), "wire_substr", wire);
    }

    return body;
}

std::string CSharpGenerator::deserialize_bitfield(uint32_t tag, const member_desc_t& member, Cursor& cursor)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const std::string var = "unit_" + std::to_string(place.unit);
    const std::string cstype = cs_type(std::get<member_type>(member));

    std::string body;
    if (place.first)
    {
        const unsigned int bytes = wire_bytes(unit.type);
        body += cursor.at(bytes);
        body += "        ulong " + var + " = " + read(bytes, unit.endian, cursor.index) + ";\n";
    }

    // Shifting the field to the top of the ulong and back down masks it, or
    // for a signed field, as a long, sign-extends it.
    const unsigned int up = 64 - place.shift - place.bits;
    const unsigned int down = 64 - place.bits;
    std::string bits = up ? "(" + var + " << " + std::to_string(up) + ")" : var;
    if (is_signed_cs(cstype))
    {
        bits = "unchecked((long)" + bits + ")";
    }
    if (down)
    {
        bits = "(" + bits + " >> " + std::to_string(down) + ")";
    }
    if (cstype != "ulong" && cstype != "long")
    {
        bits = "unchecked((" + cstype + ")" + bits + ")";
    }
    body += "        " + field_name(std::get<member_name>(member)) + " = " + bits + ";\n";

    return body;
}

// The setter keeps only the field's bits: unsigned fields are masked, signed
// ones shifted to the top of their C# type (int for the narrow ones) and back
// to sign-extend them.
std::string CSharpGenerator::set_bitfield(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string cstype = cs_type(type);
    const unsigned int bits = std::get<member_bits>(member);

    std::string kept = "value";
    if (bits < bitfields::width(type))
    {
        if (false == is_signed_cs(cstype))
        {
            const std::string suffix = cstype == "ulong" ? "UL" : cstype == "uint" ? "U"
                                                                                   : "";
            kept = "value & " + bitfields::hex(bitfields::mask(bits)) + suffix;
        }
        else
        {
            const std::string unused = std::to_string((cstype == "long" ? 64 : 32) - bits);
            kept = "(value << " + unused + ") >> " + unused;
        }

        if (cstype == "byte" || cstype == "sbyte" || cstype == "ushort" || cstype == "short")
        {
            kept = "unchecked((" + cstype + ")(" + kept + "))";
        }
    }

    return subst(replace(value("property_bitfield_str"), "wire_substr", kept), member);
}

}
