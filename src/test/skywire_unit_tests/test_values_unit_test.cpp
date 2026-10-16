//
//   Copyright (C) 2026 Skyloupe, LLC
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <protozero/pbf_writer.hpp>
#include "skywire/test_values.h"

using namespace skyloupe;

namespace
{

std::vector<uint8_t> bytes_of(const std::string& s)
{
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::vector<uint8_t> encode_pb(const std::string& type, uint32_t tag, unsigned int length = 0)
{
    std::vector<uint8_t> out;
    EXPECT_TRUE(test_values::encode_pb(type, tag, length, out)) << type;
    return out;
}

}

// encode_pb must agree with protozero, which the C++ PZ stream uses, for the
// test value of every native type.
TEST(TestValuesUnitTest, EncodePbMatchesProtozero)
{
    std::string buffer;
    protozero::pbf_writer pbf{buffer};

    pbf.add_int32(1, 65);
    ASSERT_EQ(encode_pb("char", 1), bytes_of(buffer));

    buffer.clear();
    pbf.add_uint32(2, 200);
    ASSERT_EQ(encode_pb("unsigned char", 2), bytes_of(buffer));

    buffer.clear();
    pbf.add_uint32(3, 201);
    ASSERT_EQ(encode_pb("uint8_t", 3), bytes_of(buffer));

    buffer.clear();
    pbf.add_int32(4, -50);
    ASSERT_EQ(encode_pb("int8_t", 4), bytes_of(buffer));

    buffer.clear();
    pbf.add_uint32(5, 60000);
    ASSERT_EQ(encode_pb("uint16_t", 5), bytes_of(buffer));

    buffer.clear();
    pbf.add_int32(6, -12345);
    ASSERT_EQ(encode_pb("int16_t", 6), bytes_of(buffer));

    buffer.clear();
    pbf.add_uint32(7, 3000000000u);
    ASSERT_EQ(encode_pb("uint32_t", 7), bytes_of(buffer));

    buffer.clear();
    pbf.add_int32(8, -2000000000);
    ASSERT_EQ(encode_pb("int32_t", 8), bytes_of(buffer));

    buffer.clear();
    pbf.add_uint64(9, 12000000000ull);
    ASSERT_EQ(encode_pb("uint64_t", 9), bytes_of(buffer));

    buffer.clear();
    pbf.add_int64(10, -9000000000ll);
    ASSERT_EQ(encode_pb("int64_t", 10), bytes_of(buffer));

    buffer.clear();
    pbf.add_float(11, 3.5f);
    ASSERT_EQ(encode_pb("float", 11), bytes_of(buffer));

    buffer.clear();
    pbf.add_double(12, 3.14159);
    ASSERT_EQ(encode_pb("double", 12), bytes_of(buffer));

    buffer.clear();
    pbf.add_bytes(13, "skywire");
    ASSERT_EQ(encode_pb("string", 13, 7), bytes_of(buffer));
}

TEST(TestValuesUnitTest, EncodePbKeySizes)
{
    for (uint32_t tag : { 1u, 15u, 16u, 2047u, 2048u, 18999u, 20000u, 536870911u })
    {
        std::string buffer;
        protozero::pbf_writer pbf{buffer};
        pbf.add_uint32(tag, 3000000000u);
        ASSERT_EQ(encode_pb("uint32_t", tag), bytes_of(buffer)) << "tag " << tag;
    }
}

TEST(TestValuesUnitTest, EncodePbNegativeInt32IsTenByteVarint)
{
    // key, then -2000000000 sign-extended to 64 bits: 10 bytes.
    ASSERT_EQ(encode_pb("int32_t", 1).size(), 11u);
    ASSERT_EQ(encode_pb("int16_t", 1).size(), 11u);
    ASSERT_EQ(encode_pb("int8_t", 1).size(), 11u);
}

TEST(TestValuesUnitTest, EncodePbStrings)
{
    // Cut to the member length...
    ASSERT_EQ(encode_pb("string", 1, 3), (std::vector<uint8_t>{ 0x0a, 0x03, 's', 'k', 'y' }));
    // ...and without the padding a longer member holds.
    ASSERT_EQ(encode_pb("string", 1, 16), (std::vector<uint8_t>{ 0x0a, 0x07, 's', 'k', 'y', 'w', 'i', 'r', 'e' }));
}

TEST(TestValuesUnitTest, EncodePbUserTypeHasNoTestValue)
{
    std::vector<uint8_t> out;
    ASSERT_FALSE(test_values::encode_pb("OtherMessage", 1, 0, out));
    ASSERT_TRUE(out.empty());
}

TEST(TestValuesUnitTest, TextprotoValues)
{
    ASSERT_EQ(test_values::textproto_value("int8_t", 0), "-50");
    ASSERT_EQ(test_values::textproto_value("double", 0), "3.14159");
    ASSERT_EQ(test_values::textproto_value("string", 3), "\"sky\"");
    ASSERT_EQ(test_values::textproto_value("string", 16), "\"skywire\"");
    ASSERT_EQ(test_values::textproto_value("OtherMessage", 0), "");
}
