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

class CResultsUnitTest : public ::testing::Test
{
protected:
    CResultsUnitTest() {}
};

TEST_F(CResultsUnitTest, SerDes)
{
    StructyMcStructface structy, structz;
    Cstream cs;

    structy.a_myfloat100 = static_cast<float>(101.1);
    structy.b_myuint32_200 = 200;
    structy.c_myint64_201 = 201;
    structy.d_myDoubleBubble202 = (double)202.2;

    cstream_init(&cs, 1024 * 64);

    StructyMcStructface_serialize(&structy, &cs);

    StructyMcStructface_deserialize(&structz, &cs);

    ASSERT_EQ(1, StructyMcStructface_Equals(&structy, &structz));

    cstream_fini(&cs);
}

TEST_F(CResultsUnitTest, Size)
{
    StructyMcStructface structy;

    structy.a_myfloat100 = static_cast<float>(101.1);
    structy.b_myuint32_200 = 200;
    structy.c_myint64_201 = 201;
    structy.d_myDoubleBubble202 = 202.2;

    ASSERT_EQ(StructyMcStructface_size(), 24);
}