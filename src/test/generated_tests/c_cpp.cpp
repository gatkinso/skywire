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
};
 
TEST_F(CandCpp, SerDes)
{
    const int NUM = 1000;
    StructyMcStructface structy[NUM];
    Cstream cs;
    cstream_init(&cs, 1024);

    for (uint32_t i = 0; i < NUM; i++)
    {
        structy[i].a_myfloat100 = static_cast<float>(i);
        structy[i].b_myuint32_200 = i;
        structy[i].c_myint64_201 = i;
        structy[i].d_myDoubleBubble202 = (double)i;
        structy[i].e_charcoal = i%256;

        StructyMcStructface_Serialize(&structy[i], &cs);
    }

    skyloupe::Stream str(cs.size_);
    str.write(cs.buffer_, cs.size_);

    testspace::ClassyMcClassface classy[NUM];
    for (uint32_t i = 0; i < NUM; i++)
    {
        bool success = classy[i].deserialize(str);
        ASSERT_TRUE(success);

        ASSERT_EQ(structy[i].a_myfloat100, classy[i].Get_a_myfloat100());
        ASSERT_EQ(structy[i].b_myuint32_200, classy[i].Get_b_myuint32_200());
        ASSERT_EQ(structy[i].c_myint64_201, classy[i].Get_c_myint64_201());
        ASSERT_EQ(structy[i].d_myDoubleBubble202, classy[i].Get_d_myDoubleBubble202());
        ASSERT_EQ(structy[i].e_charcoal, classy[i].Get_e_charcoal());
    }

    cstream_fini(&cs);
}
