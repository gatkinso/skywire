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
#include <cstring>
#include <map>
#include "test_values.h"
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

bool host_is_little_endian()
{
    const uint16_t probe = 1;
    uint8_t first;
    std::memcpy(&first, &probe, 1);
    return first == 1;
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

    if (endian == SW_NATIVE_ENDIAN_FLAG)
    {
        endian = host_is_little_endian() ? SW_LITTLE_ENDIAN_FLAG : SW_BIG_ENDIAN_FLAG;
    }

    const unsigned int width = it->second.width;
    for (unsigned int i = 0; i < width; i++)
    {
        const unsigned int shift = (endian == SW_LITTLE_ENDIAN_FLAG) ? i : (width - 1 - i);
        out.push_back(static_cast<uint8_t>(it->second.bits >> (8 * shift)));
    }

    return true;
}

}
}
