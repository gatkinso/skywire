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
#include "ClassyMcClassface.h"

TEST(Cpp, Serialize)
{
    testspace::ClassyMcClassface classy;
    skyloupe::Stream stream;

    bool success = classy.serialize(stream);
    ASSERT_TRUE(success);
}

TEST(Cpp, Deserialize)
{
    testspace::ClassyMcClassface in, out;
    skyloupe::Stream stream;

    in.Set_a_myfloat100(10.0);
    in.Set_b_myuint32_200(200);
    in.Set_c_myint64_201(201);
    in.Set_d_myDoubleBubble202(20.2);
    in.Set_e_charcoal(30);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);
}

TEST(Cpp, Size)
{
    testspace::ClassyMcClassface classy;

    ASSERT_EQ(classy.size(), sizeof(float) + sizeof(uint32_t) + sizeof(int64_t) + sizeof(double) + sizeof(char));
}

TEST(Cpp, Equality)
{
    testspace::ClassyMcClassface in, out;
    skyloupe::Stream stream;

    in.Set_a_myfloat100(10.0);
    in.Set_b_myuint32_200(200);
    in.Set_c_myint64_201(201);
    in.Set_d_myDoubleBubble202(20.2);
    in.Set_e_charcoal(30);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);

    ASSERT_EQ(in, out);
}