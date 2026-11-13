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

#include <gtest/gtest.h>
#include "skywire/bitfields.h"
#include "skywire/test_values.h"
#include "sw_endian.h"

using namespace skyloupe;

namespace
{

bitfields::field_t bf(uint32_t tag, const char* type, unsigned int bits, unsigned int endian = 0)
{
    return { tag, type, endian, bits };
}

}

TEST(Bitfields, LsbPacksFromTheLowBits)
{
    auto layout = bitfields::layout({ bf(1, "uint8_t", 4), bf(2, "uint8_t", 3), bf(3, "uint8_t", 1) }, false);
    ASSERT_EQ(1u, layout.units.size());
    EXPECT_EQ(0u, layout.places[1].shift);
    EXPECT_EQ(4u, layout.places[2].shift);
    EXPECT_EQ(7u, layout.places[3].shift);
    EXPECT_TRUE(layout.places[1].first);
    EXPECT_FALSE(layout.places[2].first);
    EXPECT_FALSE(layout.places[2].last);
    EXPECT_TRUE(layout.places[3].last);
}

TEST(Bitfields, MsbPacksFromTheHighBits)
{
    auto layout = bitfields::layout({ bf(1, "uint16_t", 5), bf(2, "uint16_t", 6) }, true);
    ASSERT_EQ(1u, layout.units.size());
    EXPECT_EQ(11u, layout.places[1].shift);
    EXPECT_EQ(5u, layout.places[2].shift);
}

TEST(Bitfields, NewUnitWhenFieldDoesNotFit)
{
    auto layout = bitfields::layout({ bf(1, "uint16_t", 10), bf(2, "uint16_t", 10) }, false);
    ASSERT_EQ(2u, layout.units.size());
    EXPECT_EQ(0u, layout.places[2].shift);
    EXPECT_EQ(1u, layout.places[2].unit);
    EXPECT_TRUE(layout.places[1].first && layout.places[1].last);
}

TEST(Bitfields, NewUnitOnTypeOrEndianChange)
{
    auto layout = bitfields::layout({ bf(1, "uint8_t", 2), bf(2, "int8_t", 2), bf(3, "int8_t", 2, 1), bf(4, "int8_t", 2, 1) }, false);
    ASSERT_EQ(3u, layout.units.size());
    EXPECT_EQ((std::vector<uint32_t>{ 3, 4 }), layout.units[2].tags);
}

TEST(Bitfields, PlainMemberClosesTheUnit)
{
    auto layout = bitfields::layout({ bf(1, "uint8_t", 2), bf(2, "uint8_t", 0), bf(3, "uint8_t", 2) }, false);
    ASSERT_EQ(2u, layout.units.size());
    EXPECT_EQ(0u, layout.places.count(2));
    EXPECT_EQ(0u, layout.places[3].shift);
}

TEST(Bitfields, FitMasksAndSignExtends)
{
    EXPECT_EQ(0x5u, bitfields::fit("uint8_t", 3, 0xfd));
    EXPECT_EQ(0xfdu, bitfields::fit("int8_t", 3, 0x05));   // 0b101 = -3
    EXPECT_EQ(0x0002u, bitfields::fit("int16_t", 3, 0x02));
    EXPECT_EQ(0xffffffffffffffffull, bitfields::fit("int64_t", 1, 1));
    EXPECT_EQ(0x123456789ull, bitfields::fit("uint64_t", 64, 0x123456789ull));
}

TEST(Bitfields, TestValuesFit)
{
    EXPECT_EQ("5", test_values::literal_bits("uint8_t", 3));
    EXPECT_EQ("-3", test_values::literal_bits("int8_t", 3));
    EXPECT_EQ("-1", test_values::literal_bits("int32_t", 1));
    EXPECT_EQ("-9223372036854775807", test_values::literal_bits("int64_t", 64));
    EXPECT_EQ("5726623061", test_values::literal_bits("uint64_t", 33));
}

TEST(Bitfields, EncodeUnit)
{
    // 0b0101 at 0 and 0b101 at 4: 0x55, then 0b01 at 0 of a big-endian uint16_t.
    std::vector<uint8_t> out;
    ASSERT_TRUE(test_values::encode_unit("uint8_t", SW_BIG_ENDIAN_FLAG, { { 0, 4 }, { 4, 3 } }, out));
    ASSERT_TRUE(test_values::encode_unit("uint16_t", SW_BIG_ENDIAN_FLAG, { { 14, 2 } }, out));
    // -3 in 3 bits at shift 1 of a little-endian int16_t: 0b1010 = 0x000a.
    ASSERT_TRUE(test_values::encode_unit("int16_t", SW_LITTLE_ENDIAN_FLAG, { { 1, 3 } }, out));
    EXPECT_EQ((std::vector<uint8_t>{ 0x55, 0x40, 0x00, 0x0a, 0x00 }), out);
}
