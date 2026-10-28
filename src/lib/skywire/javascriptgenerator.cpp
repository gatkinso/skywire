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

#include <iostream>
#include <map>
#include <set>
#include <string>
#include "javascriptgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

namespace
{

// Names a module may not bind (reserved words, and eval and arguments in
// strict mode code), plus the globals the generated module uses: a class
// named after one would shadow it.
const std::set<std::string> reserved_bindings = {
    "await", "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do",
    "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", "implements", "import",
    "in", "instanceof", "interface", "let", "new", "null", "package", "private", "protected", "public",
    "return", "static", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while",
    "with", "yield", "eval", "arguments", "undefined", "NaN", "Infinity",
    "Array", "ArrayBuffer", "BigInt", "DataView", "Math", "Number", "Object", "RangeError", "TextDecoder",
    "TextEncoder", "TypeError", "Uint16Array", "Uint8Array",
    // The module's own helpers.
    "SW_NATIVE_LE", "swEncoder", "swDecoder", "swView", "swRequire", "swCheck", "swSetBytes", "swPutBytes",
    "swGetBytes", "swBytesEqual", "swText", "swHex"
};

// What the generated class defines, or must not shadow on its instances:
// Object.prototype's methods, and then (a getter named then makes every
// message a thenable, which await would try to resolve).
const std::set<std::string> generated_names = {
    "constructor", "size", "serialize", "serializeInto", "deserialize", "deserializeFrom", "fromBytes",
    "equals", "toJSON", "toString", "SIZE", "prototype", "__proto__", "then",
    "valueOf", "toLocaleString", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable",
    "__defineGetter__", "__defineSetter__", "__lookupGetter__", "__lookupSetter__"
};

bool spec_error(int& errorcode, const std::string& what)
{
    errorcode |= SW_SPEC_INVALID;
    std::cerr << "Invalid specification for JavaScript: " << what << std::endl;
    return false;
}

std::string replace_all(std::string text, const std::string& token, const std::string& with)
{
    if (token.empty())
    {
        return text;
    }
    for (auto pos = text.find(token); pos != std::string::npos; pos = text.find(token, pos + with.size()))
    {
        text.replace(pos, token.size(), with);
    }
    return text;
}

}

std::string JavaScriptGenerator::view_type(const std::string& type)
{
    static const std::map<std::string, std::string> views = {
        { "char", "Int8" }, { "unsigned char", "Uint8" }, { "uint8_t", "Uint8" }, { "int8_t", "Int8" },
        { "uint16_t", "Uint16" }, { "int16_t", "Int16" },
        { "uint32_t", "Uint32" }, { "int32_t", "Int32" }, { "float", "Float32" },
        { "uint64_t", "BigUint64" }, { "int64_t", "BigInt64" }, { "double", "Float64" }
    };

    auto it = views.find(type);
    return it == views.end() ? "" : it->second;
}

unsigned int JavaScriptGenerator::wire_bytes(const std::string& type)
{
    const std::string view = view_type(type);
    if (view.empty())
        return 0;
    if (view.find("8") != std::string::npos)
        return 1;
    if (view.find("16") != std::string::npos)
        return 2;
    if (view.find("32") != std::string::npos)
        return 4;
    return 8;
}

bool JavaScriptGenerator::is_bigint(const std::string& type)
{
    return view_type(type).compare(0, 3, "Big") == 0;
}

std::string JavaScriptGenerator::field_name(const std::string& name)
{
    return "_" + name;
}

std::string JavaScriptGenerator::text_name(const std::string& name)
{
    return name + "Text";
}

std::string JavaScriptGenerator::module_path(const std::string& type)
{
    std::string pattern = lang_spec_.get_value_string("outputimplfilename", "@NOTFOUND");
    if (pattern == "@NOTFOUND" || pattern.empty())
    {
        pattern = "{classname}.js";
    }
    return "./" + replace_all(pattern, "{classname}", type);
}

std::string JavaScriptGenerator::value(const std::string& key)
{
    return lang_spec_.get_value_string(key);
}

std::string JavaScriptGenerator::block(const std::string& block_id, const std::string& body)
{
    std::string block_str = value(block_id);
    block_str = replace_all(block_str, value("classname_substr"), classname_);
    return replace_all(block_str, value("body_token"), body);
}

std::string JavaScriptGenerator::subst(std::string template_str, const member_desc_t& member)
{
    const std::string& name = std::get<member_name>(member);
    const std::string& type = std::get<member_type>(member);

    template_str = replace_all(template_str, value("classname_substr"), classname_);
    template_str = replace_all(template_str, value("member_substr"), name);
    template_str = replace_all(template_str, value("field_substr"), field_name(name));
    template_str = replace_all(template_str, value("text_substr"), text_name(name));
    template_str = replace_all(template_str, value("view_substr"), view_type(type));
    template_str = replace_all(template_str, value("bits_substr"), std::to_string(std::get<member_bits>(member)));
    template_str = replace_all(template_str, value("length_substr"), std::to_string(std::get<member_length>(member)));
    // A nested message's type; native ones substitute their own.
    template_str = replace_all(template_str, value("type_substr"), type);

    return template_str;
}

// The DataView littleEndian argument; single bytes take none.
std::string JavaScriptGenerator::endian_arg(unsigned int endian_flag)
{
    if (endian_flag == SW_BIG_ENDIAN_FLAG)
        return value("big_endian_str");
    else if (endian_flag == SW_LITTLE_ENDIAN_FLAG)
        return value("little_endian_str");
    return value("native_endian_str");
}

std::string JavaScriptGenerator::at(unsigned int displacement)
{
    const std::string local = value("offset_local_str");
    return displacement == 0 ? local : local + " + " + std::to_string(displacement);
}

bool JavaScriptGenerator::generate(int& errorcode)
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

    ss_.str("");
    if (false == do_declarations(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

// Every property the class has must be unique within it: the members'
// accessors (and a string's text getter) and backing fields, and what the
// generated code defines. Type names become module bindings.
bool JavaScriptGenerator::check_names(int& errorcode)
{
    bool valid = true;

    std::vector<std::string> types = { classname_ };
    for (auto& type : user_types())
    {
        types.push_back(type);
    }
    for (auto& type : types)
    {
        if (reserved_bindings.count(type) != 0)
        {
            valid = spec_error(errorcode, "type name '" + type + "' is reserved in a JavaScript module");
        }
    }

    std::map<std::string, std::string> taken; // property name -> member that has it
    for (auto& name : generated_names)
    {
        taken[name] = "";
    }

    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        const std::string& type = std::get<member_type>(member.second);

        if (is_native_type(type) && false == is_string_type(type) && view_type(type).empty())
        {
            valid = spec_error(errorcode, "member '" + name + "' has type " + type + ", which has no JavaScript equivalent");
        }

        std::vector<std::string> declared = { name, field_name(name) };
        if (is_string_type(type))
        {
            declared.push_back(text_name(name));
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

bool JavaScriptGenerator::do_implementation(int& errorcode)
{
    const std::string separator = value("block_separator_str");

    ss_ << value("banner_str");

    const std::string imports = imp_imports();
    if (false == imports.empty())
    {
        ss_ << separator << imports;
    }

    const std::string constants = imp_constants();
    if (false == constants.empty())
    {
        ss_ << separator << constants;
    }

    ss_ << separator << imp_class();
    ss_ << imp_size();
    ss_ << imp_helpers();

    results_["implementation"] = ss_.str();

    return true;
}

// The spec's includes are imported for their side effects; each nested
// message type is imported from its own module.
std::string JavaScriptGenerator::imp_imports()
{
    std::string out;

    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
    for (auto item : values)
    {
        if (item == "@NOTFOUND" || item.empty())
        {
            continue;
        }
        // Includes may be written "x" or <x>, for C and C++.
        if (item.size() >= 2 && ((item.front() == '"' && item.back() == '"') || (item.front() == '<' && item.back() == '>')))
        {
            item = item.substr(1, item.size() - 2);
        }
        out += replace_all(value("import_str"), value("include_substr"), item);
    }

    for (auto& type : user_types())
    {
        std::string line = replace_all(value("import_class_str"), value("type_substr"), type);
        out += replace_all(line, value("module_substr"), module_path(type));
    }

    return out;
}

std::string JavaScriptGenerator::imp_constants()
{
    std::string out;
    if (uses_native_order())
    {
        out += value("native_le_str");
    }
    if (has_strings())
    {
        out += value("text_codec_str");
    }
    return out;
}

// Whether a member of more than one byte, or a bitfield unit, is in native
// byte order.
bool JavaScriptGenerator::uses_native_order()
{
    for (auto& member : ordered_members_)
    {
        const bool bitfield = is_bitfield(member.second);
        const std::string& type = bitfield ? unit_of(member.first).type : std::get<member_type>(member.second);
        const unsigned int endian = bitfield ? unit_of(member.first).endian : std::get<member_endianness>(member.second);
        if (wire_bytes(type) > 1 && endian != SW_BIG_ENDIAN_FLAG && endian != SW_LITTLE_ENDIAN_FLAG)
        {
            return true;
        }
    }
    return false;
}

bool JavaScriptGenerator::has_strings()
{
    for (auto& member : ordered_members_)
    {
        if (is_string_type(std::get<member_type>(member.second)))
        {
            return true;
        }
    }
    return false;
}

std::string JavaScriptGenerator::imp_class()
{
    const std::string separator = value("block_separator_str");

    std::string body = block("constructor_block", imp_constructor());
    body += separator + imp_accessors();
    body += separator + block("size_fcn_block", "");
    body += separator + block("serialize_array_fcn_block", "");
    body += separator + block("serialize_fcn_block", imp_serialize());
    body += separator + block("deserialize_array_fcn_block", "");
    body += separator + block("deserialize_fcn_block", imp_deserialize());
    body += separator + block("from_bytes_fcn_block", "");
    body += separator + block("equals_fcn_block", imp_equals());
    body += separator + block("tojson_fcn_block", imp_tojson());
    body += separator + block("tostring_fcn_block", imp_tostring());

    return block("class_block", body);
}

std::string JavaScriptGenerator::imp_constructor()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "member_string_init_str";
        else if (is_user_type(type))
            key = "member_user_init_str";
        else if (is_bigint(type))
            key = "member_bigint_init_str";
        else
            key = "member_init_str";

        body += subst(value(key), member.second);
    }

    return body;
}

// The value a setter stores: v wrapped to the member's type, as typed arrays
// and C do, or for a bitfield cut to its width.
std::string JavaScriptGenerator::setter_wrap(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string v = value("setter_value_str");
    const unsigned int bits = std::get<member_bits>(member);

    if (bits == 0 || bits >= bitfields::width(type))
    {
        return replace_all(value("wrap_" + view_type(type) + "_str"), value("value_substr"), v);
    }

    const std::string count = std::to_string(bits);
    if (is_bigint(type))
    {
        return std::string(bitfields::is_signed(type) ? "BigInt.asIntN(" : "BigInt.asUintN(") + count + ", BigInt(" + v + "))";
    }
    if (bitfields::is_signed(type))
    {
        const std::string unused = std::to_string(32 - bits);
        return "(" + v + " << " + unused + ") >> " + unused;
    }
    // A mask below 2^31 leaves the result non-negative.
    return v + " & " + bitfields::hex(bitfields::mask(bits));
}

// Accessors are separated by a blank line where either has a doc comment.
std::string JavaScriptGenerator::imp_accessors()
{
    const std::string separator = value("block_separator_str");

    std::string body;
    bool last_documented = false;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string accessor;

        if (is_string_type(type))
            accessor = subst(value("accessor_string_str"), member.second);
        else if (is_user_type(type))
            accessor = subst(value("accessor_user_str"), member.second);
        else
        {
            const std::string key = is_bitfield(member.second) ? "accessor_bitfield_str" : "accessor_str";
            accessor = subst(replace_all(value(key), value("wire_substr"), setter_wrap(member.second)), member.second);
        }

        const bool documented = accessor.find("/**") != std::string::npos;
        if (false == body.empty() && (documented || last_documented))
        {
            body += separator;
        }
        body += accessor;
        last_documented = documented;
    }

    return body;
}

// SIZE is a constant plus each nested message's SIZE, which its module has
// defined by the time this one runs.
std::string JavaScriptGenerator::imp_size()
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
        else if (is_user_type(type))
            terms.push_back(subst(value("size_user_mem_str"), member.second));
        else
            fixed += wire_bytes(type);
    }

    std::string size = std::to_string(fixed);
    if (fixed == 0 && false == terms.empty())
    {
        size.clear();
    }
    for (auto& term : terms)
    {
        size += (size.empty() ? "" : separator) + term;
    }

    return replace_all(block("size_define_str", ""), value("size_substr"), size);
}

// Fixed-size members go at constant displacements from offset; a nested
// message returns the offset after it, from which the next members count.
std::string JavaScriptGenerator::imp_serialize()
{
    unsigned int displacement = 0;
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string code;

        if (is_bitfield(member.second))
        {
            body += serialize_bitfield(member.first, member.second, displacement);
            continue;
        }
        else if (is_string_type(type))
        {
            code = value("serialize_string_mem_str");
            code = replace_all(code, value("offset_substr"), at(displacement));
            displacement += std::get<member_length>(member.second);
        }
        else if (is_user_type(type))
        {
            code = replace_all(value("serialize_user_mem_str"), value("offset_substr"), at(displacement));
            displacement = 0;
        }
        else
        {
            const unsigned int bytes = wire_bytes(type);
            code = replace_all(value("serialize_mem_str"), value("offset_substr"), at(displacement));
            code = replace_all(code, value("wire_substr"), value("field_wire_str"));
            code = replace_all(code, value("endian_substr"), bytes == 1 ? "" : endian_arg(std::get<member_endianness>(member.second)));
            displacement += bytes;
        }

        body += subst(code, member.second);
    }
    body += replace_all(value("return_str"), value("offset_substr"), at(displacement));

    return body;
}

std::string JavaScriptGenerator::imp_deserialize()
{
    unsigned int displacement = 0;
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string code;

        if (is_bitfield(member.second))
        {
            body += deserialize_bitfield(member.first, member.second, displacement);
            continue;
        }
        else if (is_string_type(type))
        {
            code = replace_all(value("deserialize_string_mem_str"), value("offset_substr"), at(displacement));
            displacement += std::get<member_length>(member.second);
        }
        else if (is_user_type(type))
        {
            code = replace_all(value("deserialize_user_mem_str"), value("offset_substr"), at(displacement));
            displacement = 0;
        }
        else
        {
            const unsigned int bytes = wire_bytes(type);
            std::string read = replace_all(value("read_str"), value("offset_substr"), at(displacement));
            read = replace_all(read, value("endian_substr"), bytes == 1 ? "" : endian_arg(std::get<member_endianness>(member.second)));
            code = replace_all(value("deserialize_mem_str"), value("wire_substr"), read);
            displacement += bytes;
        }

        body += subst(code, member.second);
    }
    body += replace_all(value("return_str"), value("offset_substr"), at(displacement));

    return body;
}

// A storage unit is a local, unit_<n>: a Number for units of up to 32 bits,
// a BigInt for 64-bit ones. Its first field declares it, each field ors
// itself in, and the last one writes it.
std::string JavaScriptGenerator::serialize_bitfield(uint32_t tag, const member_desc_t& member, unsigned int& displacement)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const unsigned int bytes = wire_bytes(unit.type);
    const bool big = is_bigint(unit.type);
    const std::string var = "unit_" + std::to_string(place.unit);
    const std::string field = "this." + field_name(std::get<member_name>(member));

    std::string body;
    if (place.first)
    {
        body += "    let " + var + " = " + (big ? "0n" : "0") + ";\n";
    }

    std::string bits;
    if (big)
    {
        bits = "BigInt.asUintN(" + std::to_string(place.bits) + ", " + field + ")";
        if (place.shift)
            bits = "(" + bits + " << " + std::to_string(place.shift) + "n)";
    }
    else
    {
        bits = place.bits < 32 ? "(" + field + " & " + bitfields::hex(bitfields::mask(place.bits)) + ")" : field;
        if (place.shift)
            bits = "(" + bits + " << " + std::to_string(place.shift) + ")";
    }
    body += "    " + var + " |= " + bits + ";\n";

    if (place.last)
    {
        // DataView's setters wrap a negative unit (bit 31 set) to its bits.
        std::string code = replace_all(value("serialize_mem_str"), value("offset_substr"), at(displacement));
        code = replace_all(code, value("wire_substr"), var);
        code = replace_all(code, value("view_substr"), big ? "BigUint64" : "Uint" + std::to_string(bytes * 8));
        body += replace_all(code, value("endian_substr"), bytes == 1 ? "" : endian_arg(unit.endian));
        displacement += bytes;
    }

    return body;
}

std::string JavaScriptGenerator::deserialize_bitfield(uint32_t tag, const member_desc_t& member, unsigned int& displacement)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const bool big = is_bigint(unit.type);
    const bool is_signed = bitfields::is_signed(std::get<member_type>(member));
    const std::string var = "unit_" + std::to_string(place.unit);

    std::string body;
    if (place.first)
    {
        const unsigned int bytes = wire_bytes(unit.type);
        std::string read = replace_all(value("read_str"), value("offset_substr"), at(displacement));
        read = replace_all(read, value("view_substr"), big ? "BigUint64" : "Uint" + std::to_string(bytes * 8));
        read = replace_all(read, value("endian_substr"), bytes == 1 ? "" : endian_arg(unit.endian));
        body += "    const " + var + " = " + read + ";\n";
        displacement += bytes;
    }

    std::string bits;
    if (big)
    {
        const std::string shifted = place.shift ? "(" + var + " >> " + std::to_string(place.shift) + "n)" : var;
        bits = std::string(is_signed ? "BigInt.asIntN(" : "BigInt.asUintN(") + std::to_string(place.bits) + ", " + shifted + ")";
    }
    else if (is_signed)
    {
        // Shifting the field to the top of an int32 and back sign-extends it.
        const unsigned int up = 32 - place.shift - place.bits;
        const unsigned int down = 32 - place.bits;
        bits = up ? "(" + var + " << " + std::to_string(up) + ")" : var;
        bits = down ? bits + " >> " + std::to_string(down) : bits + " | 0";
    }
    else
    {
        bits = place.shift ? "(" + var + " >>> " + std::to_string(place.shift) + ")" : var;
        bits = place.bits < 32 ? bits + " & " + bitfields::hex(bitfields::mask(place.bits)) : bits + " >>> 0";
    }
    body += "    this." + field_name(std::get<member_name>(member)) + " = " + bits + ";\n";

    return body;
}

std::string JavaScriptGenerator::imp_equals()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "equals_string_mem_str";
        else if (is_user_type(type))
            key = "equals_user_mem_str";
        else if (type == "float" || type == "double")
            key = "equals_float_mem_str";
        else
            key = "equals_mem_str";

        body += (body.empty() ? "" : value("equals_separator_str")) + subst(value(key), member.second);
    }

    return body;
}

std::string JavaScriptGenerator::imp_tojson()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string key;

        if (is_string_type(type))
            key = "tojson_string_mem_str";
        else if (is_user_type(type))
            key = "tojson_user_mem_str";
        else if (is_bigint(type))
            key = "tojson_bigint_mem_str";
        else
            key = "tojson_mem_str";

        body += subst(value(key), member.second);
    }

    return body;
}

std::string JavaScriptGenerator::imp_tostring()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string key = is_string_type(std::get<member_type>(member.second)) ? "tostring_string_mem_str" : "tostring_mem_str";
        body += subst(replace_all(value(key), value("separator_substr"), body.empty() ? "" : ", "), member.second);
    }

    return body;
}

// The module's private helpers, only those this class uses.
std::string JavaScriptGenerator::imp_helpers()
{
    const std::string separator = value("block_separator_str");

    std::string body = separator + block("view_fcn_block", "");
    body += separator + block("require_fcn_block", "");
    if (false == user_types().empty())
    {
        body += separator + block("check_fcn_block", "");
    }
    if (has_strings())
    {
        body += separator + block("bytes_fcn_block", "");
    }

    return body;
}

// The declarations TypeScript checks users' code against: the same class,
// with each accessor's types.
bool JavaScriptGenerator::do_declarations(int& errorcode)
{
    const std::string separator = value("block_separator_str");

    ss_ << value("banner_str");

    std::string imports;
    for (auto& type : user_types())
    {
        std::string line = replace_all(value("import_type_str"), value("type_substr"), type);
        imports += replace_all(line, value("module_substr"), module_path(type));
    }
    if (false == imports.empty())
    {
        ss_ << separator << imports;
    }

    std::string body = dts_accessors();
    body += replace_all(block("dts_methods_str", ""), value("json_substr"), dts_json());
    ss_ << separator << block("dts_class_block", body);

    results_["header"] = ss_.str();

    return true;
}

std::string JavaScriptGenerator::dts_accessors()
{
    const std::string separator = value("block_separator_str");

    std::string body;
    bool last_documented = false;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string accessor;

        if (is_string_type(type))
            accessor = subst(value("dts_string_accessor_str"), member.second);
        else if (is_user_type(type))
            accessor = subst(value("dts_accessor_str"), member.second);
        else if (is_bigint(type))
            accessor = subst(value(is_bitfield(member.second) ? "dts_bigint_bitfield_accessor_str" : "dts_bigint_accessor_str"), member.second);
        else
        {
            const std::string key = is_bitfield(member.second) ? "dts_bitfield_accessor_str" : "dts_accessor_str";
            accessor = subst(replace_all(value(key), value("type_substr"), value("dts_number_str")), member.second);
        }

        const bool documented = accessor.find("/**") != std::string::npos;
        if (false == body.empty() && (documented || last_documented))
        {
            body += separator;
        }
        body += accessor;
        last_documented = documented;
    }

    return body;
}

// toJSON()'s return type: one property per member.
std::string JavaScriptGenerator::dts_json()
{
    std::string body;
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string json_type;

        if (is_string_type(type))
            json_type = value("dts_json_string_str");
        else if (is_user_type(type))
            json_type = value("dts_json_user_str");
        else if (is_bigint(type))
            json_type = value("dts_json_bigint_str");
        else
            json_type = value("dts_number_str");

        body += subst(replace_all(value("dts_json_mem_str"), value("type_substr"), json_type), member.second);
    }

    return body;
}

}
