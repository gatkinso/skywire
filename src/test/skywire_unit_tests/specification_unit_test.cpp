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
#include "google/protobuf/reflection.h"
#include <google/protobuf/stubs/status.h>
#include <google/protobuf/util/json_util.h>
#include "google/protobuf/util/message_differencer.h"
#include <string>
#pragma warning(disable:4800)
    #include "specification.pb.h"
#pragma warning(default:4800)
#include "specification/specification.h"
#include "sw_endian.h"

auto PBOK = google::protobuf::util::status_internal::StatusCode::kOk;

class SpecificationUnitTest : public ::testing::Test
{
protected:
    SpecificationUnitTest() 
        : includes_{ "\"math.h\"", "<string>", "\"myheader.h\"" }
    {}

    skyloupe::skywireproto::Specification spec_proto_;

    std::string includes_[3];
};

TEST_F(SpecificationUnitTest, Serialize_Json)
{
    std::string json_str;
    auto ret = google::protobuf::util::MessageToJsonString(spec_proto_, &json_str);

    ASSERT_EQ(PBOK, ret.code());
}

TEST_F(SpecificationUnitTest, ReflectionByName)
{
    google::protobuf::Message* paMessage = &spec_proto_;
    const google::protobuf::FieldDescriptor* paFieldDescriptorByName = paMessage->GetDescriptor()->FindFieldByName("includes");
    const google::protobuf::Reflection* pReflection = paMessage->GetReflection();

    google::protobuf::RepeatedFieldRef<std::string> ref =
        pReflection->GetRepeatedFieldRef<std::string>(*paMessage, paFieldDescriptorByName);

    for (int i = 0; i < ref.size(); i++)
    {
        std::string fld = pReflection->GetRepeatedString(*paMessage, paFieldDescriptorByName, i);

        ASSERT_EQ(fld, includes_[i]);
    }
}

TEST_F(SpecificationUnitTest, ReflectionByNumber)
{
    google::protobuf::Message* paMessage = &spec_proto_;
    const google::protobuf::FieldDescriptor* paFieldDescriptorByNum = paMessage->GetDescriptor()->FindFieldByNumber(100);
    const google::protobuf::Reflection* pReflection = paMessage->GetReflection();

    google::protobuf::RepeatedFieldRef<std::string> ref =
        pReflection->GetRepeatedFieldRef<std::string>(*paMessage, paFieldDescriptorByNum);

    for (int i = 0; i < ref.size(); i++)
    {
        std::string fld = pReflection->GetRepeatedString(*paMessage, paFieldDescriptorByNum, i);

        ASSERT_EQ(fld, includes_[i]);
    }
}

TEST_F(SpecificationUnitTest, read_file)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);
}

TEST_F(SpecificationUnitTest, read_file_BadJson)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("invalid_specification_unit_test.json");
    ASSERT_EQ(false, rc);
}

TEST_F(SpecificationUnitTest, read_file_EmptyFilename)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("");
    ASSERT_EQ(false, rc);
}

TEST_F(SpecificationUnitTest, read_file_NoExistFilename)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("no such file");
    ASSERT_EQ(false, rc);
}

TEST_F(SpecificationUnitTest, write_file_EmptyFilename)
{
    skyloupe::Specification spec;

    bool rc = spec.write_file("");
    ASSERT_EQ(false, rc);
}

TEST_F(SpecificationUnitTest, read_string)
{
    std::string json_str = "{\"stringValues\":{\"classname\":\"TestClass\",\"language\":\"C++\",\"namespace\":\"testspace\"}}";
    skyloupe::Specification spec;

    bool rc = spec.read_string(json_str);
    ASSERT_EQ(true, rc);
}

TEST_F(SpecificationUnitTest, write_string)
{
    std::string json_str_in = "{\"stringValues\":{\"classname\":\"TestClass\",\"language\":\"C++\",\"namespace\":\"testspace\"}}";
    std::string json_str_out;
    skyloupe::Specification stencil_left, stencil_right;

    bool rc = stencil_left.read_string(json_str_in);
    ASSERT_EQ(true, rc);

    rc = stencil_left.write_string(json_str_out);
    ASSERT_EQ(true, rc);

    stencil_right.read_string(json_str_out);

    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(stencil_left.get_proto(), stencil_right.get_proto()));
}

TEST_F(SpecificationUnitTest, read_file_Doesnt_Exist)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("nosuchfile.json");
    ASSERT_EQ(false, rc);
}

TEST_F(SpecificationUnitTest, get_value_string)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    std::string json_str;
    auto ret = google::protobuf::util::MessageToJsonString(spec.get_proto(), &json_str);

    ASSERT_EQ(PBOK, ret.code());

    std::string values = spec.get_value_string("namespace", "NOT FOUND");

    ASSERT_TRUE(values == "testspace");
}

TEST_F(SpecificationUnitTest, get_value_repeated)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    std::vector<std::string> values = spec.get_value("includes", "NOT FOUND");

    ASSERT_TRUE(values.size() == 3);

    ASSERT_TRUE(values[0] == "<string>");
    ASSERT_TRUE(values[1] == "<list>");
    ASSERT_TRUE(values[2] == "<map>");
}

TEST_F(SpecificationUnitTest, set_value_string)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    rc = spec.set_value_string("namespace", "newnamespace");
    ASSERT_EQ(true, rc);

    std::string values = spec.get_value_string("namespace", "NOT FOUND");

    ASSERT_TRUE(values == "newnamespace");
}

TEST_F(SpecificationUnitTest, clear)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    rc = spec.clear();
    ASSERT_EQ(true, rc);

    ASSERT_EQ(spec.get_proto().includes_size(), 0);
}

TEST_F(SpecificationUnitTest, clear_key)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    std::vector<std::string> values = spec.get_value("includes", "NOT FOUND");

    ASSERT_TRUE(values.size() == 3);

    rc = spec.clear_key("includes");
    ASSERT_EQ(true, rc);

    values = spec.get_value("includes", "NOT FOUND");

    ASSERT_TRUE(values.size() == 1);
    ASSERT_TRUE(values[0] == "NOT FOUND");
}

TEST_F(SpecificationUnitTest, clear_key_string)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    std::string values = spec.get_value_string("language", "NOT FOUND");

    ASSERT_TRUE(values == "c++");

    rc = spec.clear_key_string("c++");
    ASSERT_EQ(true, rc);

    values = spec.get_value_string("c++", "NOT FOUND");

    ASSERT_TRUE(values == "NOT FOUND");
}

TEST_F(SpecificationUnitTest, set_value_repeated)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    rc = spec.set_value("includes", "<new.h>");
    ASSERT_EQ(true, rc);

    std::vector<std::string> values = spec.get_value("includes", "NOT FOUND");

    ASSERT_TRUE(values.size() == 4);

    ASSERT_TRUE(values[0] == "<string>");
    ASSERT_TRUE(values[1] == "<list>");
    ASSERT_TRUE(values[2] == "<map>");
    ASSERT_TRUE(values[3] == "<new.h>");
}

TEST_F(SpecificationUnitTest, get_value_nomatch)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    std::vector<std::string> values = spec.get_value("doesntexist", "NOT FOUND");

    ASSERT_TRUE(values.size() == 1);
    ASSERT_TRUE(values[0] == "NOT FOUND");
}

TEST_F(SpecificationUnitTest, set_get_value_member)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    rc = spec.set_value_member(100, "float", "m_myfloat", SW_BIG_ENDIAN_FLAG);
    ASSERT_EQ(true, rc);

    std::string type;
    std::string name;
    uint32_t endian;
    rc = spec.get_value_member(100, type, name, endian);

    ASSERT_TRUE(rc);

    ASSERT_EQ(type, "float");
    ASSERT_EQ(name, "m_myfloat");
    ASSERT_EQ(endian, SW_BIG_ENDIAN_FLAG);
}

TEST_F(SpecificationUnitTest, clear_key_member)
{
    skyloupe::Specification spec;

    bool rc = spec.read_file("specification_unit_test.json");
    ASSERT_EQ(true, rc);

    rc = spec.set_value_member(100, "float", "m_myfloat", SW_BIG_ENDIAN_FLAG);
    ASSERT_EQ(true, rc);

    rc = spec.clear_key_member(100);

    ASSERT_TRUE(rc);

    std::string type;
    std::string name;
    uint32_t endian;
    rc = spec.get_value_member(100, type, name, endian);

    ASSERT_FALSE(rc);
}

//TEST_F(SpecificationUnitTest, GetString)
//{
//    skyloupe::Specification spec;
//
//    spec.set_value_string("language", "C++");
//    spec.set_value_string("namespace", "testspace");
//    spec.set_value_string("classname", "TestClass");
//    spec.set_value("includes", "<header1.h>");
//    spec.set_value("includes", "<header2.h>");
//    spec.set_value("includes", "<header3.h>");
//
//    bool rc = spec.set_value_member(100, "float", "m_myfloat", SW_LITTLE_ENDIAN_FLAG);
//    rc = spec.set_value_member(200, "int16_t", "m_myint", SW_LITTLE_ENDIAN_FLAG);
//
//    rc = spec.write_file("specification_unit_test_out.json");
//    ASSERT_EQ(true, rc);
//}
