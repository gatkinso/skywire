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
    StructyMcStructface structy;
    Cstream cs;

    structy.a_myfloat100 = static_cast<float>(101.1);
    structy.b_myuint32_200 = 200;
    structy.c_myint64_201 = 201;
    structy.d_myDoubleBubble202 = (double)202.2;
    structy.e_charcoal = 30;

    cstream_init(&cs, 128);

    for (uint32_t i = 0; i < 1000; i++)
    {
        StructyMcStructface_Serialize(&structy, &cs);
    }

    skyloupe::Stream str(cs.size_);
    str.write(cs.buffer_, cs.size_);

    for (uint32_t i = 0; i < 1000; i++)
    {
        testspace::ClassyMcClassface classy;
        bool success = classy.deserialize(str);
        ASSERT_TRUE(success);

        ASSERT_EQ(structy.a_myfloat100, classy.Get_a_myfloat100());
        ASSERT_EQ(structy.b_myuint32_200, classy.Get_b_myuint32_200());
        ASSERT_EQ(structy.c_myint64_201, classy.Get_c_myint64_201());
        ASSERT_EQ(structy.d_myDoubleBubble202, classy.Get_d_myDoubleBubble202());
        ASSERT_EQ(structy.e_charcoal, classy.Get_e_charcoal());
    }

    cstream_fini(&cs);
}
