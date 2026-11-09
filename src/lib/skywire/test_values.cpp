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
#include <cstdio>
#include <cstring>
#include <map>
#include "test_values.h"
#include "bitfields.h"
#include "sw_endian.h"

namespace skyloupe
{
namespace test_values
{

namespace
{

struct TestValue
{
    const char* literal;
    uint64_t bits;
    unsigned int width;
};

template <typename T, typename U>
uint64_t bits_of(T value)
{
    U u;
    static_assert(sizeof(T) == sizeof(U), "width mismatch");
    std::memcpy(&u, &value, sizeof(u));
    return u;
}

const std::map<std::string, TestValue>& values()
{
    static const std::map<std::string, TestValue> table = {
        { "char",          { "65",          static_cast<uint8_t>(65),                    1 } },
        { "unsigned char", { "200",         static_cast<uint8_t>(200),                   1 } },
        { "uint8_t",       { "201",         static_cast<uint8_t>(201),                   1 } },
        { "int8_t",        { "-50",         static_cast<uint8_t>(int8_t(-50)),           1 } },
        { "uint16_t",      { "60000",       static_cast<uint16_t>(60000),                2 } },
        { "int16_t",       { "-12345",      static_cast<uint16_t>(int16_t(-12345)),      2 } },
        { "uint32_t",      { "3000000000",  static_cast<uint32_t>(3000000000u),          4 } },
        { "int32_t",       { "-2000000000", static_cast<uint32_t>(int32_t(-2000000000)), 4 } },
        { "uint64_t",      { "12000000000", static_cast<uint64_t>(12000000000ull),       8 } },
        { "int64_t",       { "-9000000000", static_cast<uint64_t>(int64_t(-9000000000)), 8 } },
        { "float",         { "3.5",         bits_of<float, uint32_t>(3.5f),              4 } },
        { "double",        { "3.14159",     bits_of<double, uint64_t>(3.14159),          8 } },
    };
    return table;
}

bool is_signed(const std::string& type)
{
    return type == "char" || type == "int8_t" || type == "int16_t" || type == "int32_t" || type == "int64_t";
}

// The test value for a string member: string_value() cut to the member length.
std::string string_test_value(unsigned int length)
{
    const std::string value = string_value();
    return value.substr(0, std::min<size_t>(length, value.size()));
}

void append_varint(uint64_t value, std::vector<uint8_t>& out)
{
    while (value >= 0x80)
    {
        out.push_back(static_cast<uint8_t>(value | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<uint8_t>(value));
}

void append_key(uint32_t tag, unsigned int wire_type, std::vector<uint8_t>& out)
{
    append_varint((static_cast<uint64_t>(tag) << 3) | wire_type, out);
}

bool host_is_little_endian()
{
    const uint16_t probe = 1;
    uint8_t first;
    std::memcpy(&first, &probe, 1);
    return first == 1;
}

void append_integer(uint64_t bits, unsigned int width, unsigned int endian, std::vector<uint8_t>& out)
{
    if (endian == SW_NATIVE_ENDIAN_FLAG)
    {
        endian = host_is_little_endian() ? SW_LITTLE_ENDIAN_FLAG : SW_BIG_ENDIAN_FLAG;
    }

    for (unsigned int i = 0; i < width; i++)
    {
        const unsigned int shift = (endian == SW_LITTLE_ENDIAN_FLAG) ? i : (width - 1 - i);
        out.push_back(static_cast<uint8_t>(bits >> (8 * shift)));
    }
}

}

const char* literal(const std::string& type)
{
    auto it = values().find(type);
    return it == values().end() ? nullptr : it->second.literal;
}

const char* string_value()
{
    return "skywire";
}

bool encode(const std::string& type, unsigned int endian, unsigned int length, std::vector<uint8_t>& out)
{
    if (type == "string")
    {
        const size_t used = std::min<size_t>(length, std::strlen(string_value()));
        out.insert(out.end(), string_value(), string_value() + used);
        out.insert(out.end(), length - used, 0);
        return true;
    }

    auto it = values().find(type);
    if (it == values().end())
    {
        return false;
    }

    append_integer(it->second.bits, it->second.width, endian, out);
    return true;
}

uint64_t bits_value(const std::string& type, unsigned int bits)
{
    if (false == bitfields::is_signed(type))
    {
        return bitfields::fit(type, bits, 0x5555555555555555ull);
    }

    // -(2^(bits-1)) + 1, or -1 for a single bit.
    const uint64_t negative = bits == 1 ? ~uint64_t(0) : (~uint64_t(0) << (bits - 1)) + 1;
    return bitfields::fit(type, bits, negative);
}

std::string literal_bits(const std::string& type, unsigned int bits)
{
    const uint64_t value = bits_value(type, bits);
    const unsigned int width = bitfields::width(type);

    if (bitfields::is_signed(type) && (value >> (width - 1)) != 0)
    {
        const uint64_t magnitude = (~value + 1) & bitfields::mask(width);
        return "-" + std::to_string(magnitude);
    }

    return std::to_string(value);
}

bool encode_unit(const std::string& type, unsigned int endian, const std::vector<bitfield_t>& fields, std::vector<uint8_t>& out)
{
    const unsigned int width = bitfields::width(type);
    if (width == 0)
    {
        return false;
    }

    uint64_t unit = 0;
    for (auto& field : fields)
    {
        unit |= (bits_value(type, field.bits) & bitfields::mask(field.bits)) << field.shift;
    }

    append_integer(unit, width / 8, endian, out);
    return true;
}

bool encode_pb(const std::string& type, uint32_t tag, unsigned int length, std::vector<uint8_t>& out, unsigned int bits)
{
    const unsigned int varint_type = 0, fixed64_type = 1, bytes_type = 2, fixed32_type = 5;

    if (type == "string")
    {
        std::string value = string_test_value(length);
        value.erase(value.find_last_not_of('\0') + 1);
        if (false == value.empty())
        {
            append_key(tag, bytes_type, out);
            append_varint(value.size(), out);
            out.insert(out.end(), value.begin(), value.end());
        }
        return true;
    }

    auto it = values().find(type);
    if (it == values().end())
    {
        return false;
    }

    uint64_t value = bits ? bits_value(type, bits) : it->second.bits;
    const unsigned int width = it->second.width;
    if (value == 0)
    {
        return true;
    }

    if (type == "float" || type == "double")
    {
        append_key(tag, width == 4 ? fixed32_type : fixed64_type, out);
        for (unsigned int i = 0; i < width; i++)
        {
            out.push_back(static_cast<uint8_t>(value >> (8 * i)));
        }
        return true;
    }

    if (is_signed(type) && width < 8 && (value >> (8 * width - 1)) != 0)
    {
        value |= ~uint64_t(0) << (8 * width);
    }

    append_key(tag, varint_type, out);
    append_varint(value, out);
    return true;
}

namespace
{

// The number as text: the literals are already in the canonical form (the
// shortest that reads back, JavaScript's layout), so they are used as is.
std::string number_value(const std::string& type, unsigned int bits)
{
    if (bits != 0)
    {
        return literal(type) ? literal_bits(type, bits) : "";
    }
    const char* value = literal(type);
    return value ? value : "";
}

// The string test value without its trailing NULs.
std::string used_string(unsigned int length)
{
    std::string value = string_test_value(length);
    value.erase(value.find_last_not_of('\0') + 1);
    return value;
}

}

std::string json_value(const std::string& type, unsigned int length, unsigned int bits)
{
    if (type != "string")
    {
        const std::string value = number_value(type, bits);
        const bool quoted = type == "int64_t" || type == "uint64_t";
        return value.empty() || false == quoted ? value : "\"" + value + "\"";
    }

    std::string quoted = "\"";
    for (unsigned char c : used_string(length))
    {
        if (c == '"' || c == '\\')
        {
            quoted += '\\';
            quoted += static_cast<char>(c);
        }
        else if (c < 0x20 || c > 0x7e)
        {
            char escape[8];
            std::snprintf(escape, sizeof(escape), "\\u%04x", c);
            quoted += escape;
        }
        else
        {
            quoted += static_cast<char>(c);
        }
    }
    quoted += "\"";
    return quoted;
}

std::string xml_value(const std::string& type, unsigned int length, bool& hex, unsigned int bits)
{
    hex = false;
    if (type != "string")
    {
        return number_value(type, bits);
    }

    const std::string value = used_string(length);
    for (unsigned char c : value)
    {
        if (c < 0x20 || c > 0x7e)
        {
            hex = true;
        }
    }

    std::string text;
    for (unsigned char c : value)
    {
        if (hex)
        {
            char digits[4];
            std::snprintf(digits, sizeof(digits), "%02x", c);
            text += digits;
        }
        else if (c == '<')
            text += "&lt;";
        else if (c == '>')
            text += "&gt;";
        else if (c == '&')
            text += "&amp;";
        else if (c == '"')
            text += "&quot;";
        else if (c == '\'')
            text += "&apos;";
        else
            text += static_cast<char>(c);
    }
    return text;
}

std::string textproto_value(const std::string& type, unsigned int length, unsigned int bits)
{
    if (bits != 0)
    {
        return literal(type) ? literal_bits(type, bits) : "";
    }

    if (type != "string")
    {
        const char* value = literal(type);
        return value ? value : "";
    }

    std::string quoted = "\"";
    for (unsigned char c : string_test_value(length))
    {
        if (c == '"' || c == '\\')
        {
            quoted += '\\';
            quoted += static_cast<char>(c);
        }
        else if (c < 0x20 || c > 0x7e)
        {
            char octal[8];
            std::snprintf(octal, sizeof(octal), "\\%03o", c);
            quoted += octal;
        }
        else
        {
            quoted += static_cast<char>(c);
        }
    }
    quoted += "\"";
    return quoted;
}

}
}
