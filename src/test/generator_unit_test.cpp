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
#include "skywire/generators.h"

class GeneratorUnitTest : public ::testing::Test
{
protected:
    GeneratorUnitTest() {}
    skyloupe::Stencil stencil;
};

TEST_F(GeneratorUnitTest, Instantiate)
{
    ASSERT_NO_THROW(skyloupe::CppGenerator unit(stencil));
}

TEST_F(GeneratorUnitTest, generatecpp)
{
    skyloupe::CppGenerator unit(stencil);

    //std::string json_str = "{}";
    //int errorcode = -1;

    //bool success = unit.generate_from_file(json_str, errorcode);

    //ASSERT_TRUE(success);
    //ASSERT_EQ(errorcode, 0);
}

//TEST_F(GeneratorUnitTest, generate_from_str)
//{
//    skyloupe::Generator unit(stencil);
//
//    //std::string json_str = "{}";
//    //int errorcode = -1;
//
//    //bool success = unit.generate_from_str(json_str, errorcode);
//
//    //ASSERT_TRUE(success);
//    //ASSERT_EQ(errorcode, 0);
//}
