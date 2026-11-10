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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace skyloupe
{

// Values the generated round-trip tests assign to each member, shared by the
// test generators so every language tests the same message. The expected wire
// bytes are computed here, independently of any generated code.
namespace test_values
{

// Numeric literal for a native type (e.g. "-50"), or nullptr for strings and
// user-defined types. The literals are valid in Go, Python, Swift and C.
const char* literal(const std::string& type);

// Value used for string members, truncated or zero-padded to the member length.
const char* string_value();

// Appends the expected wire bytes of the test value for one member. Native
// endianness means the byte order of the host running wiregen. Returns false
// for types with no test value (user-defined types).
bool encode(const std::string& type, unsigned int endian, unsigned int length, std::vector<uint8_t>& out);

// Appends the canonical proto3 encoding of the test value for one member,
// using tag as the field number: integers as varints (signed ones
// sign-extended to 64 bits), float/double as fixed32/fixed64, strings as bytes
// with trailing NULs trimmed. Zero values and empty strings are omitted, as
// proto3 does. Returns false for types with no test value (user-defined types).
// A bitfield member (bits != 0) uses bits_value instead of the type's value.
bool encode_pb(const std::string& type, uint32_t tag, unsigned int length, std::vector<uint8_t>& out, unsigned int bits = 0);

// The test value for one member in protobuf text format: a numeric literal,
// or a quoted, escaped string. Empty for types with no test value.
std::string textproto_value(const std::string& type, unsigned int length, unsigned int bits = 0);

// The test value for one member as it appears in the JSON or XML text form
// (docs/text-formats.md): JSON quotes 64-bit integers and strings, XML
// escapes strings or, if they hold other than printable ASCII, gives them in
// hex (with hex set true). Empty for types with no test value. A bitfield
// member (bits != 0) uses bits_value.
std::string json_value(const std::string& type, unsigned int length, unsigned int bits = 0);
std::string xml_value(const std::string& type, unsigned int length, bool& hex, unsigned int bits = 0);

// Appends the BSON element (docs/text-formats.md) for one member named name
// holding its test value: int32 up to 32-bit and uint16_t types, int64 for
// uint32_t and int64_t, int64 or (above 2^63-1) Decimal128 for uint64_t,
// double for float and double, string (or binary, if not UTF-8 or holding a
// NUL) for strings. False for types with no test value. A bitfield member
// (bits != 0) uses bits_value.
bool encode_bson(const std::string& type, const std::string& name, unsigned int length, std::vector<uint8_t>& out, unsigned int bits = 0);

// Appends a BSON element of each kind, for building documents in tests.
void bson_int32(const std::string& name, int32_t value, std::vector<uint8_t>& out);
void bson_int64(const std::string& name, int64_t value, std::vector<uint8_t>& out);
void bson_double(const std::string& name, double value, std::vector<uint8_t>& out);
// Decimal128 holding the integer magnitude high:low (a 128-bit coefficient,
// exponent 0), negative if negative.
void bson_decimal128(const std::string& name, uint64_t high, uint64_t low, bool negative, std::vector<uint8_t>& out);
void bson_string(const std::string& name, const std::string& value, std::vector<uint8_t>& out);
void bson_binary(const std::string& name, const std::string& value, std::vector<uint8_t>& out);
void bson_bool(const std::string& name, bool value, std::vector<uint8_t>& out);
// Wraps elements as a document: length, elements, terminating NUL.
std::vector<uint8_t> bson_document(const std::vector<uint8_t>& elements);

// The test value of a bitfield member of bits bits, as the type's bit
// pattern: alternating 0101... for unsigned types, so both ends of each
// field are exercised, and a negative value for signed ones.
uint64_t bits_value(const std::string& type, unsigned int bits);

// bits_value as a decimal literal (e.g. "-3"), valid in every language.
std::string literal_bits(const std::string& type, unsigned int bits);

struct bitfield_t
{
    unsigned int shift;
    unsigned int bits;
};

// Appends the expected wire bytes of one storage unit of the given type and
// endian, holding each field's bits_value at its shift.
bool encode_unit(const std::string& type, unsigned int endian, const std::vector<bitfield_t>& fields, std::vector<uint8_t>& out);

}

}
