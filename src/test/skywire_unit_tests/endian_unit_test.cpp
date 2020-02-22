//
//   Copyright (C) 2017 Skyloupe, LLC
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
#include <cstring>
#include "sw_endian.h"

class EndianUnitTest : public ::testing::Test
{
protected:
    EndianUnitTest() {}
};

TEST_F(EndianUnitTest, 16UbitToBE)
{
    unsigned char buffer[2] = { 0xAB, 0xCD };
    uint16_t myint = 0xABCD;
    
    uint16_t be = htobe16(myint);

    int cmp = std::memcmp(&be, buffer, sizeof(uint16_t));

    ASSERT_EQ(cmp, 0);
}

TEST_F(EndianUnitTest, 16UbitToLE)
{
    unsigned char buffer[2] = { 0xCD, 0xAB };
    uint16_t myint = 0xABCD;

    uint16_t le = htole16(myint);

    int cmp = std::memcmp(&le, buffer, sizeof(uint16_t));

    ASSERT_EQ(cmp, 0);
}

TEST_F(EndianUnitTest, 32UbitToBE)
{
    unsigned char buffer[4] = { 0xCA, 0xFE, 0xBA, 0xBE };
    uint32_t myint = 0xCAFEBABE;

    uint32_t be = htobe32(myint);

    int cmp = std::memcmp(&be, buffer, sizeof(uint32_t));

    ASSERT_EQ(cmp, 0);
}

TEST_F(EndianUnitTest, 32UbitToLE)
{
    unsigned char buffer[4] = { 0xBE, 0xBA, 0xFE, 0xCA };
    uint32_t myint = 0xCAFEBABE;

    uint32_t le = htole32(myint);

    int cmp = std::memcmp(&le, buffer, sizeof(uint32_t));

    ASSERT_EQ(cmp, 0);
}

TEST_F(EndianUnitTest, 64UbitToBE)
{
    unsigned char buffer[8] = { 0xFF, 0xEE, 0xEE, 0xDD, 0xFF, 0xAA, 0xCC, 0xEE };
    uint64_t myint = 0xFFEEEEDDFFAACCEE;

    uint64_t be = htobe64(myint);

    int cmp = std::memcmp(&be, buffer, sizeof(uint64_t));

    ASSERT_EQ(cmp, 0);
}

TEST_F(EndianUnitTest, 64UbitToLE)
{
    unsigned char buffer[8] = { 0xEE, 0xCC, 0xAA, 0xFF, 0xDD, 0xEE, 0xEE, 0xFF };
    uint64_t myint = 0xFFEEEEDDFFAACCEE;

    uint64_t be = htole64(myint);

    int cmp = std::memcmp(&be, buffer, sizeof(uint64_t));

    ASSERT_EQ(cmp, 0);
}
