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

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace skyloupe
{

// Bitfield members: integer members with a "bits" width, packed with their
// neighbours into storage units on the wire. Walking the members in tag
// order, a bitfield joins the open unit if it has the unit's type and endian
// and fits in the bits left; otherwise it opens a new unit. Any other member
// closes the open unit. A unit is written as one integer of its type, in its
// endian; unused bits are written as 0 and ignored on read. With "lsb" bit
// order the first field takes the low bits, with "msb" the high ones.
namespace bitfields
{

// Width in bits of a type that may hold a bitfield (uint8_t ... int64_t),
// or 0 for any other type.
unsigned int width(const std::string& type);

bool is_signed(const std::string& type);

// The unsigned type of the same width ("int16_t" -> "uint16_t").
std::string unsigned_type(const std::string& type);

// The low bits bits set.
uint64_t mask(unsigned int bits);

// value cut to bits bits and, for signed types, sign-extended to the type's
// width; the result is the type's bit pattern.
uint64_t fit(const std::string& type, unsigned int bits, uint64_t value);

// Hex literal, e.g. "0x1f".
std::string hex(uint64_t value);

// A C/C++ expression of type `type` holding expr cut to bits bits and, if
// signed, sign-extended (the setters' and deserializers' masking).
std::string c_fit(const std::string& type, unsigned int bits, const std::string& expr);

struct field_t
{
    uint32_t tag;
    std::string type;
    unsigned int endian;
    unsigned int bits;  // 0: not a bitfield
};

struct unit_t
{
    std::string type;
    unsigned int endian;
    std::vector<uint32_t> tags;  // in wire order
};

struct place_t
{
    size_t unit;  // index into layout_t::units
    unsigned int shift;
    unsigned int bits;
    bool first;  // first field of its unit in wire order
    bool last;
};

struct layout_t
{
    std::vector<unit_t> units;
    std::map<uint32_t, place_t> places;  // by tag, bitfields only
};

// fields in wire order. Fields with a type width() doesn't know, or bits out
// of range, are treated as plain members (validation rejects them).
layout_t layout(const std::vector<field_t>& fields, bool msb);

}

}
