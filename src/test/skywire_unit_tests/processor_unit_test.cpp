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
#include "skywire/processor.h"

using namespace skyloupe;
std::string msg_str =
"{                                                    \
   \"stringValues\":{                                 \
      \"classname\":\"TestClass\",                    \
      \"outputheaderfilename\":\"TestClass++.h\",     \
      \"outputtestfilename\":\"TestClassTest++.cpp\", \
      \"namespace\":\"testspace\",                    \
      \"language\": \"c++\",                          \
      \"lang_file\": \"lang.cpp.json\"                \
   },                                                 \
   \"members\":{                                      \
      \"1\":{                                         \
         \"type\":\"float\",                          \
         \"name\":\"float_1\",                        \
         \"endian\":1                                 \
      }                                               \
   },                                                 \
   \"includes\":[                                     \
      \"<streamable.h>\",                             \
      \"<stdint.h>\"                                  \
   ]                                                  \
}";

std::string lang_str =
"{                                                                                                                                        \
    \"stringValues\": {                                                                                                                   \
        \"banner_str\":\"// This is the generated go file\n\",                                                                            \
        \"include_substr\":  \"__INCLUDEFILE__\",                                                                                         \
        \"structname_substr\":\"__STRUCTNAME__\",                                                                                         \
        \"type_substr\":\"__TYPE__\",                                                                                                     \
        \"member_substr\":\"__MEMBERNAME__\",                                                                                             \
        \"format_substr\":\"__FORMAT__\",                                                                                                 \
        \"index_substr\":\"__INDEX__\",                                                                                                   \
        \"endian_substr\":\"__ENDIAN__\",                                                                                                 \
        \"namespace_substr\":\"__NAMESPACE__\",                                                                                           \
        \"indent_str\":\"\t\",                                                                                                            \
        \"block_separator_str\":\"\n\",                                                                                                   \
        \"body_token\": \"__BLOCK_BODY__\",                                                                                               \
        \"namespace_block\": \"package __NAMESPACE__\n\",                                                                                 \
        \"import_block\": \"import (\n__BLOCK_BODY__)\n\",                                                                                \
        \"var_block\": \"var (\n__BLOCK_BODY__)\n\",                                                                                      \
        \"struct_block\": \"type __STRUCTNAME__ struct {\n__BLOCK_BODY__}\n\",                                                            \
        \"serialize_fcn_block\": \"//Serialize method\nfunc (s *__STRUCTNAME__) Serialize() (o []byte) {\n__BLOCK_BODY__}\n\",            \
        \"deserialize_fcn_block\": \"//Deserialize method\nfunc (s *__STRUCTNAME__) Deserialize(o []byte) (error) {\n__BLOCK_BODY__}\n\", \
        \"equals_fcn_block\": \"//Equals method\nfunc (s *__STRUCTNAME__) Equals(other *__STRUCTNAME__) (bool) {\n__BLOCK_BODY__}\n\",    \
        \"size_fcn_block\": \"//Size method\nfunc (s *__STRUCTNAME__) Size() (uint64) {\n__BLOCK_BODY__;\n}\n\",                          \
        \"member_decl_str\":\"\t__MEMBERNAME__ __TYPE__\n\",                                                                              \
        \"equal_eval_start_str\":\"\tif\n\",                                                                                              \
        \"equal_eval_member_str\":\"\t\ts.__MEMBERNAME__ == other.__MEMBERNAME__\",                                                       \
        \"equal_eval_tail_str\":\" {\n\t\treturn true\n\t} else {\n\t\t return false\n\t}\n\",                                            \
        \"size_eval_start_str\":\"\treturn \",                                                                                            \
        \"size_eval_member_str\":\"unsafe.SizeOf(s.__MEMBERNAME__)\",                                                                     \
        \"char\": \"int8\",                                                                                                               \
        \"unsigned char\" : \"uint8\",                                                                                                    \
        \"uint8_t\" : \"uint8\",                                                                                                          \
        \"int8_t\" : \"int8\",                                                                                                            \
        \"uint16_t\" : \"uint16\",                                                                                                        \
        \"int16_t\" : \"int16\",                                                                                                          \
        \"uint32_t\" : \"uint32\",                                                                                                        \
        \"int32_t\" : \"int32\",                                                                                                          \
        \"uint64_t\" : \"uint64\",                                                                                                        \
        \"int64_t\" : \"int64\",                                                                                                          \
        \"float\" : \"float32\",                                                                                                          \
        \"double\" : \"float64\",                                                                                                         \
    },                                                                                                                                    \
}";

class ProcessorUnitTest : public ::testing::Test
{
protected:
    ProcessorUnitTest() {}

    std::string input_msg_str;
    std::string input_lang_str;
    results_t output_str_map; 
    std::string outputdir;
    int errorcode = SW_SUCCESS;
    bool generate_tests = false;
    bool write_to_file = false;

    std::string msgpathname;
    std::string langpathname;
};

TEST_F(ProcessorUnitTest, Instantiate)
{
    ASSERT_NO_THROW(Processor p);
}

TEST_F(ProcessorUnitTest, ProcessString_BlankMsg)
{
    Processor p;

    bool ret = p.process_string(input_msg_str, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTSTRING);
}

TEST_F(ProcessorUnitTest, ProcessString_BlankLang)
{
    Processor p;

    input_msg_str = "not blank";

    bool ret = p.process_string(input_msg_str, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTSTRING);
}

TEST_F(ProcessorUnitTest, ProcessString_BadMsg)
{
    Processor p;

    input_msg_str = "invalid";
    input_lang_str = "not blank";

    bool ret = p.process_string(input_msg_str, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_STRINGREADERROR);
}

TEST_F(ProcessorUnitTest, ProcessString_BadLang)
{
    Processor p;

    input_msg_str = msg_str;
    input_lang_str = "invalid";

    bool ret = p.process_string(input_msg_str, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_STRINGREADERROR);
}

TEST_F(ProcessorUnitTest, ProcessString_Success)
{
    Processor p;

    input_msg_str = msg_str;
    input_lang_str = lang_str;

    bool ret = p.process_string(input_msg_str, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_TRUE(ret);
    ASSERT_EQ(errorcode, SW_SUCCESS);
}

TEST_F(ProcessorUnitTest, ProcessFile_BlankMsgFilenamne)
{
    Processor p;

    bool ret = p.process_file(msgpathname, langpathname, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTFILENAME);
}

TEST_F(ProcessorUnitTest, ProcessFile_BlankLangFilenamne)
{
    Processor p;

    msgpathname = "not blank";

    bool ret = p.process_file(msgpathname, langpathname, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTFILENAME);
}




