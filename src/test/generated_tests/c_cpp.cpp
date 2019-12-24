//
//   Copyright (C) 2018 Skyloupe, LLC
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
#include "StructyMcStructface.h"
#include "ClassyMcClassface.h"

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

    int sign = -1;
    for (uint32_t i = 0; i < NUM; i++)
    {
        sign *= -1;
        structy[i].a_myfloat100 = static_cast<float>(i);
        structy[i].b_myuint32_200 = i;
        structy[i].c_myint64_201 = i;
        structy[i].d_myDoubleBubble202 = (double)i;
        structy[i].e_charcoal = i%256;
        structy[i].f_myfloat1000 = sign*i*66.6f;
        structy[i].g_myuint32_2000 = i*49;
        structy[i].h_myint64_2010 = sign*i*-80;
        structy[i].i_myDoubleBubble2020 = sign*i*109.8;

        ASSERT_EQ(0, StructyMcStructface_Serialize(&structy[i], &cs));
    }

    skyloupe::Stream str(cs.size_);
    str.write(cs.buffer_, cs.size_);

    for (uint32_t i = 0; i < NUM; i++)
    {
        bool success = classy[i].deserialize(str);
        ASSERT_TRUE(success);

        ASSERT_EQ(structy[i].a_myfloat100, classy[i].Get_a_myfloat100());
        ASSERT_EQ(structy[i].b_myuint32_200, classy[i].Get_b_myuint32_200());
        ASSERT_EQ(structy[i].c_myint64_201, classy[i].Get_c_myint64_201());
        ASSERT_EQ(structy[i].d_myDoubleBubble202, classy[i].Get_d_myDoubleBubble202());
        ASSERT_EQ(structy[i].e_charcoal, classy[i].Get_e_charcoal());
        ASSERT_EQ(structy[i].f_myfloat1000, classy[i].Get_f_myfloat1000());
        ASSERT_EQ(structy[i].g_myuint32_2000, classy[i].Get_g_myuint32_2000());
        ASSERT_EQ(structy[i].h_myint64_2010, classy[i].Get_h_myint64_2010());
        ASSERT_EQ(structy[i].i_myDoubleBubble2020, classy[i].Get_i_myDoubleBubble2020());
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

    const int i = 107;
    StructyMcStructface structy;
    testspace::ClassyMcClassface classy;

    structy.a_myfloat100 = static_cast<float>(i);
    structy.b_myuint32_200 = i;
    structy.c_myint64_201 = i;
    structy.d_myDoubleBubble202 = (double)i;
    structy.e_charcoal = i % 256;
    structy.f_myfloat1000 = i*66.6f;
    structy.g_myuint32_2000 = i * 49;
    structy.h_myint64_2010 = i*-80;
    structy.i_myDoubleBubble2020 = i*109.8;

    classy.Set_a_myfloat100(static_cast<float>(structy.a_myfloat100));
    classy.Set_b_myuint32_200(structy.b_myuint32_200);
    classy.Set_c_myint64_201(structy.c_myint64_201);
    classy.Set_d_myDoubleBubble202(static_cast<double>(structy.d_myDoubleBubble202));
    classy.Set_e_charcoal(structy.e_charcoal);
    classy.Set_f_myfloat1000(structy.f_myfloat1000);
    classy.Set_g_myuint32_2000(structy.g_myuint32_2000);
    classy.Set_h_myint64_2010(structy.h_myint64_2010);
    classy.Set_i_myDoubleBubble2020(structy.i_myDoubleBubble2020);

    ASSERT_EQ(0, StructyMcStructface_Serialize(&structy, &cs));
    ASSERT_EQ(true, classy.serialize(str));
    ASSERT_EQ(str.byte_size(), StructyMcStructface_Size());
    ASSERT_EQ(str.byte_size(), testspace::ClassyMcClassface::size());
    ASSERT_EQ(0, std::memcmp(cs.buffer_, str.data(), str.byte_size()));

    cstream_fini(&cs);
}