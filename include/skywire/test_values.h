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
bool encode_pb(const std::string& type, uint32_t tag, unsigned int length, std::vector<uint8_t>& out);

// The test value for one member in protobuf text format: a numeric literal,
// or a quoted, escaped string. Empty for types with no test value.
std::string textproto_value(const std::string& type, unsigned int length);

}

}
