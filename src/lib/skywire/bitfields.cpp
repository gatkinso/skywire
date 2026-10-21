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

#include <cstdio>
#include "bitfields.h"

namespace skyloupe
{
namespace bitfields
{

unsigned int width(const std::string& type)
{
    if (type == "uint8_t" || type == "int8_t") return 8;
    if (type == "uint16_t" || type == "int16_t") return 16;
    if (type == "uint32_t" || type == "int32_t") return 32;
    if (type == "uint64_t" || type == "int64_t") return 64;
    return 0;
}

bool is_signed(const std::string& type)
{
    return width(type) != 0 && type[0] == 'i';
}

std::string unsigned_type(const std::string& type)
{
    return is_signed(type) ? "u" + type : type;
}

uint64_t mask(unsigned int bits)
{
    return bits >= 64 ? ~uint64_t(0) : (uint64_t(1) << bits) - 1;
}

uint64_t fit(const std::string& type, unsigned int bits, uint64_t value)
{
    value &= mask(bits);
    if (is_signed(type) && bits < 64 && (value >> (bits - 1)) != 0)
    {
        value |= ~mask(bits);
    }
    return value & mask(width(type));
}

std::string hex(uint64_t value)
{
    char text[24];
    std::snprintf(text, sizeof(text), "0x%llx", static_cast<unsigned long long>(value));
    return text;
}

std::string c_fit(const std::string& type, unsigned int bits, const std::string& expr)
{
    const std::string suffix = width(type) == 64 ? "ull" : "u";
    const std::string u = unsigned_type(type);

    if (bits >= width(type))
    {
        return "(" + type + ")(" + expr + ")";
    }

    const std::string masked = "((" + u + ")(" + expr + ") & " + hex(mask(bits)) + suffix + ")";
    if (false == is_signed(type))
    {
        return "(" + type + ")" + masked;
    }

    // (v ^ sign) - sign sign-extends the low bits bits of v.
    const std::string sign = hex(uint64_t(1) << (bits - 1)) + suffix;
    return "(" + type + ")(" + u + ")((" + masked + " ^ " + sign + ") - " + sign + ")";
}

layout_t layout(const std::vector<field_t>& fields, bool msb)
{
    layout_t out;
    bool open = false;
    unsigned int used = 0;

    for (auto& field : fields)
    {
        const unsigned int unit_width = width(field.type);
        if (field.bits == 0 || unit_width == 0 || field.bits > unit_width)
        {
            open = false;
            continue;
        }

        if (open)
        {
            const unit_t& unit = out.units.back();
            if (unit.type != field.type || unit.endian != field.endian || used + field.bits > unit_width)
            {
                open = false;
            }
        }

        if (false == open)
        {
            if (false == out.units.empty())
            {
                out.places[out.units.back().tags.back()].last = true;
            }
            out.units.push_back({ field.type, field.endian, {} });
            open = true;
            used = 0;
        }

        unit_t& unit = out.units.back();
        place_t place;
        place.unit = out.units.size() - 1;
        place.shift = msb ? unit_width - used - field.bits : used;
        place.bits = field.bits;
        place.first = unit.tags.empty();
        place.last = false;
        out.places[field.tag] = place;
        unit.tags.push_back(field.tag);
        used += field.bits;
    }

    if (false == out.units.empty())
    {
        out.places[out.units.back().tags.back()].last = true;
    }

    return out;
}

}
}
