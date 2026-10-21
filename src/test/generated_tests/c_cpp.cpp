//
//   Copyright (C) 2018 Geoffrey Atkinson
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
#include <new>
#include <vector>
#include "StructyMcStructface.h"
#include "ClassyMcClassface.h"

// Fills every member with a value that varies with i, so each message in a
// run is different.
static void fill(StructyMcStructface& s, uint32_t i)
{
    const int sign = (i % 2) ? -1 : 1;

    std::memset(&s, 0, sizeof(s));
    s.a_mydouble100 = sign*i*109.8;
    s.b_myuint32_200 = i*49;
    s.c_myint64_201 = sign*static_cast<int64_t>(i)*-80;
    s.d_myfloat202 = sign*i*66.6f;
    s.e_myint16_203 = static_cast<int16_t>(sign*static_cast<int>(i));
    s.f_myuint16_204 = static_cast<uint16_t>(i*61);
    s.g_myint32_205 = sign*static_cast<int32_t>(i)*1001;
    s.h_myuint64_206 = static_cast<uint64_t>(i)*12000000000ull;
    s.i_charcoal301 = static_cast<char>(i%128);
    strncpy(s.j_mystring302, "skywire", 16);
    s.k_uchar303 = static_cast<unsigned char>(i%256);
    s.l_myuint8_304 = static_cast<uint8_t>(255 - i%256);
    s.m_myint8_305 = static_cast<int8_t>(-sign*static_cast<int>(i%128));
    strncpy(s.n_shortstring306, "skywire", 4);
    s.o_myfloat1000 = static_cast<float>(i);
    s.p_myuint32_2000 = i;
    s.q_myint64_2010 = -static_cast<int64_t>(i);
    s.r_mydouble2020 = static_cast<double>(i)/3;
}

static void copy(testspace::ClassyMcClassface& c, const StructyMcStructface& s)
{
    c.Set_a_mydouble100(s.a_mydouble100);
    c.Set_b_myuint32_200(s.b_myuint32_200);
    c.Set_c_myint64_201(s.c_myint64_201);
    c.Set_d_myfloat202(s.d_myfloat202);
    c.Set_e_myint16_203(s.e_myint16_203);
    c.Set_f_myuint16_204(s.f_myuint16_204);
    c.Set_g_myint32_205(s.g_myint32_205);
    c.Set_h_myuint64_206(s.h_myuint64_206);
    c.Set_i_charcoal301(s.i_charcoal301);
    c.Set_j_mystring302("skywire");
    c.Set_k_uchar303(s.k_uchar303);
    c.Set_l_myuint8_304(s.l_myuint8_304);
    c.Set_m_myint8_305(s.m_myint8_305);
    c.Set_n_shortstring306("skywire");
    c.Set_o_myfloat1000(s.o_myfloat1000);
    c.Set_p_myuint32_2000(s.p_myuint32_2000);
    c.Set_q_myint64_2010(s.q_myint64_2010);
    c.Set_r_mydouble2020(s.r_mydouble2020);
}

static void expect_same(const StructyMcStructface& s, const testspace::ClassyMcClassface& c)
{
    EXPECT_EQ(s.a_mydouble100, c.Get_a_mydouble100());
    EXPECT_EQ(s.b_myuint32_200, c.Get_b_myuint32_200());
    EXPECT_EQ(s.c_myint64_201, c.Get_c_myint64_201());
    EXPECT_EQ(s.d_myfloat202, c.Get_d_myfloat202());
    EXPECT_EQ(s.e_myint16_203, c.Get_e_myint16_203());
    EXPECT_EQ(s.f_myuint16_204, c.Get_f_myuint16_204());
    EXPECT_EQ(s.g_myint32_205, c.Get_g_myint32_205());
    EXPECT_EQ(s.h_myuint64_206, c.Get_h_myuint64_206());
    EXPECT_EQ(s.i_charcoal301, c.Get_i_charcoal301());
    EXPECT_EQ(0, std::memcmp(s.j_mystring302, c.Get_j_mystring302(), 16));
    EXPECT_EQ(s.k_uchar303, c.Get_k_uchar303());
    EXPECT_EQ(s.l_myuint8_304, c.Get_l_myuint8_304());
    EXPECT_EQ(s.m_myint8_305, c.Get_m_myint8_305());
    EXPECT_EQ(0, std::memcmp(s.n_shortstring306, c.Get_n_shortstring306(), 4));
    EXPECT_EQ(s.o_myfloat1000, c.Get_o_myfloat1000());
    EXPECT_EQ(s.p_myuint32_2000, c.Get_p_myuint32_2000());
    EXPECT_EQ(s.q_myint64_2010, c.Get_q_myint64_2010());
    EXPECT_EQ(s.r_mydouble2020, c.Get_r_mydouble2020());
}

class CandCpp : public ::testing::Test
{
protected:
    CandCpp() {}

    static const int NUM = 1000;
    StructyMcStructface structy[NUM] = {};
    testspace::ClassyMcClassface classy[NUM] = {};
};
 
TEST_F(CandCpp, SerDes)
{
    Cstream cs;
    cstream_init(&cs, 1024);

    for (uint32_t i = 0; i < NUM; i++)
    {
        fill(structy[i], i);
        ASSERT_EQ(0, StructyMcStructface_Serialize(&structy[i], &cs));
    }

    const size_t bytes = cstream_available(&cs);
    ASSERT_EQ(bytes, NUM*StructyMcStructface_Size());

    skyloupe::Stream str(bytes);
    str.write(cs.head_, bytes);

    for (uint32_t i = 0; i < NUM; i++)
    {
        ASSERT_TRUE(classy[i].deserialize(str));
        expect_same(structy[i], classy[i]);
    }

    cstream_fini(&cs);
}

TEST_F(CandCpp, SizeTest)
{
    ASSERT_EQ(StructyMcStructface_Size(), testspace::ClassyMcClassface::size());
}

TEST_F(CandCpp, StreamCompare)
{
    Cstream cs;
    cstream_init(&cs, StructyMcStructface_Size());
    skyloupe::Stream str(cs.size_);

    StructyMcStructface structy;
    testspace::ClassyMcClassface classy;

    fill(structy, 107);
    copy(classy, structy);

    ASSERT_EQ(0, StructyMcStructface_Serialize(&structy, &cs));
    ASSERT_EQ(true, classy.serialize(str));
    ASSERT_EQ(str.byte_size(), StructyMcStructface_Size());
    ASSERT_EQ(str.byte_size(), testspace::ClassyMcClassface::size());
    ASSERT_EQ(0, std::memcmp(cs.head_, str.data(), str.byte_size()));

    cstream_fini(&cs);
}

TEST_F(CandCpp, CStringSetterStopsAtTerminator)
{
    // The setter used to memcpy the full field length from a shorter string,
    // reading past it and putting whatever followed onto the wire.
    StructyMcStructface s;
    std::memset(&s, 0x5a, sizeof(s));

    ASSERT_EQ(0, Set_StructyMcStructface_j_mystring302(&s, "abc"));

    char expected[16] = { 'a', 'b', 'c' };
    ASSERT_EQ(0, std::memcmp(s.j_mystring302, expected, sizeof(expected)));
}

TEST_F(CandCpp, CppDefaultConstructedIsZero)
{
    // Members are value-initialized, so a fresh object serializes as zeros
    // rather than whatever was in memory.
    alignas(testspace::ClassyMcClassface) unsigned char storage[sizeof(testspace::ClassyMcClassface)];
    std::memset(storage, 0x5a, sizeof(storage));
    auto* c = new (storage) testspace::ClassyMcClassface();

    skyloupe::Stream str;
    ASSERT_TRUE(c->serialize(str));

    std::vector<char> zeros(str.byte_size(), 0);
    ASSERT_EQ(0, std::memcmp(str.data(), zeros.data(), zeros.size()));
    c->~ClassyMcClassface();
}

