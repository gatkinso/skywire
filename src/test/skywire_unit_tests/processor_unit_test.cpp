//
//   Copyright (C) 2017 Geoffrey Atkinson
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
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>
#include "skywire/processor.h"
#include "skywire/skywire.h"

using namespace skyloupe;
std::string msg_str =
"{                                                    \
   \"stringValues\":{                                 \
      \"classname\":\"TestClass\",                    \
      \"outputheaderfilename\":\"TestClass++.h\",     \
      \"outputtestfilename\":\"TestClassTest++.cpp\", \
      \"namespace\":\"testspace\"                     \
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
        \"language\": \"c++\",                                                                                                            \
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
    \"native_types\": [ \"char\", \"unsigned char\", \"uint8_t\", \"int8_t\", \"uint16_t\", \"int16_t\", \"uint32_t\", \"int32_t\",       \
                      \"uint64_t\", \"int64_t\", \"float\", \"double\", \"string\" ],                                                     \
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
    bool generate_relay = false;
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

    bool ret = p.process_file(msgpathname, langpathname, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTFILENAME);
}

TEST_F(ProcessorUnitTest, ProcessFile_BlankLangFilenamne)
{
    Processor p;

    msgpathname = "not blank";

    bool ret = p.process_file(msgpathname, langpathname, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTFILENAME);
}

TEST_F(ProcessorUnitTest, ProcessFileLangstr_BlankMsgFilename)
{
    Processor p;

    bool ret = p.process_file_langstr(msgpathname, lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTFILENAME);
}

TEST_F(ProcessorUnitTest, ProcessFileLangstr_BlankLang)
{
    Processor p;

    msgpathname = "not blank";

    bool ret = p.process_file_langstr(msgpathname, input_lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file);
    ASSERT_FALSE(ret);
    ASSERT_EQ(errorcode, SW_BLANKINPUTSTRING);
}

// A minimal c++ message spec; each validation test swaps in one bad piece.
static std::string make_spec(const std::string& members,
                             const std::string& extra_values = "",
                             const std::string& includes = R"(["<stdint.h>"])")
{
    return R"({"stringValues":{"classname":"TestClass","namespace":"testspace",)"
           R"("outputheaderfilename":"TestClass.h")" + extra_values + R"(},"members":)" + members +
           R"(,"includes":)" + includes + "}";
}

static const std::string good_members = R"({"1":{"type":"float","name":"float_1","endian":1}})";

class ProcessorValidationTest : public ProcessorUnitTest
{
protected:
    bool run(const std::string& spec)
    {
        Processor p;
        return p.process_string(spec, lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file, generate_relay);
    }

    void expect_rejected(const std::string& spec)
    {
        ASSERT_FALSE(run(spec));
        ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
        ASSERT_TRUE(output_str_map.empty());
    }
};

TEST_F(ProcessorValidationTest, AcceptsValidSpec)
{
    ASSERT_TRUE(run(make_spec(good_members)));
    ASSERT_EQ(errorcode, SW_SUCCESS);
}

TEST_F(ProcessorValidationTest, AcceptsQualifiedNamespace)
{
    std::string spec = make_spec(good_members);
    spec.replace(spec.find("testspace"), 9, "com.acme.wire");
    ASSERT_TRUE(run(spec));

    spec = make_spec(good_members);
    spec.replace(spec.find("testspace"), 9, "acme::wire");
    ASSERT_TRUE(run(spec));
}

// The target language is the lang file's, never the message spec's.
TEST_F(ProcessorValidationTest, LanguageComesFromLangFile)
{
    ASSERT_TRUE(run(make_spec(good_members, R"(,"language":"swift")")));
    ASSERT_EQ(errorcode, SW_SUCCESS);
    ASSERT_EQ(output_str_map.count("header"), 1u);           // C++ writes a header,
    ASSERT_EQ(output_str_map.count("implementation"), 0u);   // Swift an implementation.
}

TEST_F(ProcessorValidationTest, RejectsLangFileWithoutLanguage)
{
    std::string lang = lang_str;
    const std::string language = R"("language": "c++",)";
    lang.erase(lang.find(language), language.size());

    Processor p;
    ASSERT_FALSE(p.process_string(make_spec(good_members), lang, output_str_map, outputdir, errorcode, generate_tests, write_to_file));
    ASSERT_EQ(errorcode, SW_UNKNOWNLANG);
    ASSERT_TRUE(output_str_map.empty());
}

// Lang file patterns are literal text: regex syntax in one is just
// characters to find, and generates the same code.
TEST_F(ProcessorValidationTest, LangPatternsAreLiteralText)
{
    ASSERT_TRUE(run(make_spec(good_members)));
    const results_t plain = output_str_map;

    std::string lang = lang_str;
    const std::string token = "__MEMBERNAME__";
    const std::string literal = "__MEMBER(NAME$&__";
    for (auto pos = lang.find(token); pos != std::string::npos; pos = lang.find(token, pos + literal.size()))
    {
        lang.replace(pos, token.size(), literal);
    }

    Processor p;
    output_str_map.clear();
    ASSERT_TRUE(p.process_string(make_spec(good_members), lang, output_str_map, outputdir, errorcode, generate_tests, write_to_file));
    ASSERT_EQ(errorcode, SW_SUCCESS);
    ASSERT_EQ(output_str_map, plain);
}

TEST_F(ProcessorValidationTest, AcceptsLangFileForExpectedLanguage)
{
    Processor p;
    p.set_language("c++");
    ASSERT_TRUE(p.process_string(make_spec(good_members), lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file));
    ASSERT_EQ(errorcode, SW_SUCCESS);
}

TEST_F(ProcessorValidationTest, RejectsLangFileForAnotherLanguage)
{
    Processor p;
    p.set_language("swift");
    ASSERT_FALSE(p.process_string(make_spec(good_members), lang_str, output_str_map, outputdir, errorcode, generate_tests, write_to_file));
    ASSERT_EQ(errorcode, SW_UNKNOWNLANG);
    ASSERT_TRUE(output_str_map.empty());
}

// version() is "major.minor.patch", the same numbers as its parts.
TEST(Version, PartsMatchString)
{
    const std::string parts = std::to_string(version_major()) + "." + std::to_string(version_minor()) + "." +
                              std::to_string(version_patch());
    ASSERT_EQ(std::string(version()), parts);
}

TEST(FindLanguage, NamesAndAliases)
{
    std::string language, lang_filename;

    ASSERT_TRUE(find_language("c++", language, lang_filename));
    ASSERT_EQ(language, "c++");
    ASSERT_EQ(lang_filename, "lang.cpp.json");

    ASSERT_TRUE(find_language("cpp", language, lang_filename));
    ASSERT_EQ(language, "c++");
    ASSERT_EQ(lang_filename, "lang.cpp.json");

    for (const char* name : { "cs", "c#" })
    {
        ASSERT_TRUE(find_language(name, language, lang_filename)) << name;
        ASSERT_EQ(language, "csharp");
        ASSERT_EQ(lang_filename, "lang.csharp.json");
    }

    ASSERT_TRUE(find_language("golang", language, lang_filename));
    ASSERT_EQ(language, "go");
    ASSERT_EQ(lang_filename, "lang.go.json");

    ASSERT_TRUE(find_language("js", language, lang_filename));
    ASSERT_EQ(language, "javascript");
    ASSERT_EQ(lang_filename, "lang.javascript.json");

    ASSERT_TRUE(find_language("python", language, lang_filename));
    ASSERT_EQ(language, "python3");
    ASSERT_EQ(lang_filename, "lang.python3.json");

    for (const char* name : { "c", "csharp", "go", "java", "javascript", "proto3", "python3", "rust", "swift" })
    {
        ASSERT_TRUE(find_language(name, language, lang_filename)) << name;
        ASSERT_EQ(language, name);
    }
}

TEST(FindLanguage, RejectsUnknown)
{
    std::string language = "unchanged", lang_filename = "unchanged";

    ASSERT_FALSE(find_language("cobol", language, lang_filename));
    ASSERT_FALSE(find_language("", language, lang_filename));
    ASSERT_FALSE(find_language("C++", language, lang_filename));
    ASSERT_EQ(language, "unchanged");
    ASSERT_EQ(lang_filename, "unchanged");
}

TEST_F(ProcessorValidationTest, RejectsPathTraversalInOutputFilename)
{
    expect_rejected(make_spec(good_members, R"(,"outputtestfilename":"../escape.cpp")"));
}

TEST_F(ProcessorValidationTest, RejectsPathSeparatorInOutputFilename)
{
    expect_rejected(make_spec(good_members, R"(,"outputtestfilename":"sub/escape.cpp")"));
}

TEST_F(ProcessorValidationTest, RejectsCodeInMemberName)
{
    expect_rejected(make_spec(R"({"1":{"type":"float","name":"x; } evil(); struct { int y"}})"));
}

TEST_F(ProcessorValidationTest, RejectsRegexFormatInMemberName)
{
    // Not an identifier, so it could break the generated code.
    expect_rejected(make_spec(R"({"1":{"type":"float","name":"a$&b"}})"));
}

TEST_F(ProcessorValidationTest, RejectsCodeInMemberType)
{
    expect_rejected(make_spec(R"({"1":{"type":"int x; int","name":"a"}})"));
}

TEST_F(ProcessorValidationTest, RejectsDuplicateMemberName)
{
    expect_rejected(make_spec(R"({"1":{"type":"float","name":"a"},"2":{"type":"float","name":"a"}})"));
}

TEST_F(ProcessorValidationTest, RejectsZeroLengthString)
{
    expect_rejected(make_spec(R"({"1":{"type":"string","name":"s","length":0}})"));
}

TEST_F(ProcessorValidationTest, RejectsUnknownEndian)
{
    expect_rejected(make_spec(R"({"1":{"type":"float","name":"a","endian":3}})"));
}

TEST_F(ProcessorValidationTest, AcceptsBitfields)
{
    ASSERT_TRUE(run(make_spec(R"({"1":{"type":"uint8_t","name":"a","bits":3},"2":{"type":"int64_t","name":"b","bits":64}})")));
    ASSERT_TRUE(run(make_spec(R"({"1":{"type":"uint16_t","name":"a","bits":1}})", R"(,"bitorder":"msb")")));
    ASSERT_TRUE(run(make_spec(R"({"1":{"type":"uint16_t","name":"a","bits":1}})", R"(,"bitorder":"lsb")")));
}

TEST_F(ProcessorValidationTest, RejectsBitsOnOtherTypes)
{
    for (const char* type : { "float", "double", "char", "unsigned char", "string", "Nested" })
    {
        SCOPED_TRACE(type);
        expect_rejected(make_spec(std::string(R"({"1":{"type":")") + type + R"(","name":"a","bits":3,"length":4}})"));
    }
}

TEST_F(ProcessorValidationTest, RejectsBitsWiderThanType)
{
    expect_rejected(make_spec(R"({"1":{"type":"uint8_t","name":"a","bits":9}})"));
    expect_rejected(make_spec(R"({"1":{"type":"int32_t","name":"a","bits":33}})"));
}

TEST_F(ProcessorValidationTest, RejectsUnknownBitorder)
{
    expect_rejected(make_spec(R"({"1":{"type":"uint8_t","name":"a","bits":3}})", R"(,"bitorder":"MSB")"));
}

static std::string member_with_tag(const std::string& tag)
{
    return R"({")" + tag + R"(":{"type":"float","name":"a"}})";
}

TEST_F(ProcessorValidationTest, AcceptsProtobufFieldNumbers)
{
    for (const char* tag : { "1", "18999", "20000", "536870911" })
    {
        SCOPED_TRACE(tag);
        ASSERT_TRUE(run(make_spec(member_with_tag(tag))));
    }
}

TEST_F(ProcessorValidationTest, RejectsTagZero)
{
    expect_rejected(make_spec(member_with_tag("0")));
}

TEST_F(ProcessorValidationTest, RejectsProtobufReservedTags)
{
    for (const char* tag : { "19000", "19500", "19999" })
    {
        SCOPED_TRACE(tag);
        output_str_map.clear();
        expect_rejected(make_spec(member_with_tag(tag)));
    }
}

TEST_F(ProcessorValidationTest, RejectsTagAboveProtobufMaximum)
{
    expect_rejected(make_spec(member_with_tag("536870912")));
}

TEST_F(ProcessorValidationTest, RejectsBadClassname)
{
    std::string spec = make_spec(good_members);
    spec.replace(spec.find("\"TestClass\""), 11, "\"Test Class\"");
    expect_rejected(spec);
}

TEST_F(ProcessorValidationTest, RejectsNewlineInInclude)
{
    expect_rejected(make_spec(good_members, "", R"(["<stdint.h>\n#include <evil.h>"])"));
}

TEST_F(ProcessorValidationTest, RejectsNoMembers)
{
    expect_rejected(make_spec("{}"));
    ASSERT_NE(errorcode & SW_LANG_NOMEMBERS, 0u);
}

TEST_F(ProcessorValidationTest, MissingOutputFilenameWritesNothing)
{
    // Tests are generated but there is no outputtestfilename: nothing may be
    // written, not even the header that does have a name.
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() / "skywire_processor_validation";
    fs::remove_all(dir);
    fs::create_directories(dir);

    outputdir = dir.string();
    generate_tests = true;
    write_to_file = true;

    ASSERT_FALSE(run(make_spec(good_members)));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
    ASSERT_TRUE(fs::is_empty(dir));

    fs::remove_all(dir);
}

TEST_F(ProcessorValidationTest, RelayOnlyWhenAskedFor)
{
    generate_tests = true;
    ASSERT_TRUE(run(make_spec(good_members, R"(,"outputtestfilename":"TestClassTest.cpp","outputrelayfilename":"relay.cpp")")));
    ASSERT_EQ(output_str_map.count("relay"), 0u);

    output_str_map.clear();
    generate_relay = true;
    ASSERT_TRUE(run(make_spec(good_members, R"(,"outputtestfilename":"TestClassTest.cpp","outputrelayfilename":"relay.cpp")")));
    ASSERT_EQ(output_str_map.count("relay"), 1u);
    ASSERT_NE(output_str_map["relay"].find("int main("), std::string::npos);
}

TEST_F(ProcessorValidationTest, RejectsPathTraversalInRelayFilename)
{
    expect_rejected(make_spec(good_members, R"(,"outputrelayfilename":"../relay.cpp")"));
}

TEST_F(ProcessorValidationTest, RelayNeedsNestedSpec)
{
    // A relay checks the members inside a nested message, so it needs that
    // message's spec, which a spec given as a string has no directory to find.
    const std::string members = R"({"1":{"type":"OtherMessage","name":"nested"}})";
    generate_tests = true;
    ASSERT_TRUE(run(make_spec(members, R"(,"outputtestfilename":"TestClassTest.cpp")")));

    output_str_map.clear();
    generate_relay = true;
    ASSERT_FALSE(run(make_spec(members, R"(,"outputtestfilename":"TestClassTest.cpp","outputrelayfilename":"relay.cpp")")));
    ASSERT_NE(errorcode & SW_FILEREADERROR, 0u);
}

// Output file names: the lang file gives a {classname} pattern for each
// output, and a name in the spec takes its place.
class ProcessorFilenameTest : public ProcessorUnitTest
{
protected:
    std::filesystem::path dir;

    void SetUp() override
    {
        // ctest runs each test in its own process, in parallel.
        dir = std::filesystem::temp_directory_path() /
              (std::string("skywire_filenames_") + ::testing::UnitTest::GetInstance()->current_test_info()->name());
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
        outputdir = dir.string();
        generate_tests = true;
        write_to_file = true;
    }

    void TearDown() override
    {
        std::filesystem::remove_all(dir);
    }

    static std::string patterned_lang()
    {
        std::string lang = lang_str;
        const std::string key = "\"language\": \"c++\",";
        lang.insert(lang.find(key) + key.size(),
                    R"("outputheaderfilename": "{classname}.h",)"
                    R"("outputtestfilename": "{classname}Test.cpp",)"
                    R"("outputrelayfilename": "{classname}Relay.cpp",)");
        return lang;
    }

    static std::string spec(const std::string& file_values = "")
    {
        return R"({"stringValues":{"classname":"TestClass","namespace":"testspace")" + file_values +
               R"(},"members":)" + good_members + R"(,"includes":["<stdint.h>"]})";
    }

    bool run(const std::string& msg)
    {
        Processor p;
        return p.process_string(msg, patterned_lang(), output_str_map, outputdir, errorcode, generate_tests, write_to_file, generate_relay);
    }

    std::set<std::string> written() const
    {
        std::set<std::string> names;
        for (const auto& entry : std::filesystem::directory_iterator(dir))
        {
            names.insert(entry.path().filename().string());
        }
        return names;
    }
};

TEST_F(ProcessorFilenameTest, NamesComeFromLangFile)
{
    ASSERT_TRUE(run(spec()));
    ASSERT_EQ(errorcode, SW_SUCCESS);
    // The relay pattern names the relay's file but does not ask for a relay.
    ASSERT_EQ(written(), (std::set<std::string>{ "TestClass.h", "TestClassTest.cpp" }));
}

TEST_F(ProcessorFilenameTest, RelayFlagAddsRelay)
{
    generate_relay = true;
    ASSERT_TRUE(run(spec()));
    ASSERT_EQ(written(), (std::set<std::string>{ "TestClass.h", "TestClassTest.cpp", "TestClassRelay.cpp" }));
}

TEST_F(ProcessorFilenameTest, RelayNeedsNoTests)
{
    generate_tests = false;
    generate_relay = true;
    ASSERT_TRUE(run(spec()));
    ASSERT_EQ(written(), (std::set<std::string>{ "TestClass.h", "TestClassRelay.cpp" }));
}

TEST_F(ProcessorFilenameTest, SpecRelayNameDoesNotAskForRelay)
{
    ASSERT_TRUE(run(spec(R"(,"outputrelayfilename":"Relay.cpp")")));
    ASSERT_EQ(written(), (std::set<std::string>{ "TestClass.h", "TestClassTest.cpp" }));
}

TEST_F(ProcessorFilenameTest, SpecNameReplacesPattern)
{
    ASSERT_TRUE(run(spec(R"(,"outputheaderfilename":"Wire.h")")));
    ASSERT_EQ(written(), (std::set<std::string>{ "Wire.h", "TestClassTest.cpp" }));
    // The test includes the header by its actual name.
    ASSERT_NE(output_str_map["unittest"].find("Wire.h"), std::string::npos);
}

TEST_F(ProcessorFilenameTest, SpecNameMayUsePattern)
{
    generate_relay = true;
    ASSERT_TRUE(run(spec(R"(,"outputheaderfilename":"{classname}_wire.h","outputrelayfilename":"{classname}_relay.cpp")")));
    ASSERT_EQ(written(), (std::set<std::string>{ "TestClass_wire.h", "TestClassTest.cpp", "TestClass_relay.cpp" }));
}

TEST_F(ProcessorFilenameTest, SnakeCaseClassnameToken)
{
    generate_relay = true;
    ASSERT_TRUE(run(spec(R"(,"outputheaderfilename":"{classname_snake}.h","outputrelayfilename":"{classname_snake}_{classname}.cpp")")));
    ASSERT_EQ(written(), (std::set<std::string>{ "test_class.h", "TestClassTest.cpp", "test_class_TestClass.cpp" }));
}

TEST_F(ProcessorFilenameTest, ExpandedNameIsStillValidated)
{
    ASSERT_FALSE(run(spec(R"(,"outputheaderfilename":"../{classname}.h")")));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
    ASSERT_TRUE(written().empty());
}

// Defaults the lang file supplies (runtime includes) and the --namespace
// override, so one spec serves every language.
class ProcessorDefaultsTest : public ProcessorUnitTest
{
protected:
    std::string name_space;

    static std::string lang(const std::string& includes = R"(["\"streamable.h\"", "<stdint.h>"])")
    {
        std::string lang = lang_str;
        const std::string key = "\"language\": \"c++\",";
        lang.insert(lang.find(key) + key.size(),
                    R"("sentry_str": "#pragma once\n",)"
                    R"("include_str": "#include __INCLUDEFILE__\n",)"
                    R"("namespace_str": "namespace __NAMESPACE__ {\n",)");
        if (false == includes.empty())
        {
            lang.insert(lang.find('{') + 1, R"("includes": )" + includes + ",");
        }
        return lang;
    }

    static std::string spec(const std::string& includes = "", const std::string& name_space = R"("namespace":"testspace",)")
    {
        return R"({"stringValues":{)" + name_space + R"("classname":"TestClass","outputheaderfilename":"TestClass.h"},)"
               R"("members":)" + good_members + (includes.empty() ? "" : R"(,"includes":)" + includes) + "}";
    }

    bool run(const std::string& msg, const std::string& lang_file = lang())
    {
        Processor p;
        p.set_namespace(name_space);
        return p.process_string(msg, lang_file, output_str_map, outputdir, errorcode, generate_tests, write_to_file, generate_relay);
    }

    // The header's #include lines, in order.
    std::vector<std::string> includes()
    {
        std::vector<std::string> lines;
        std::istringstream header(output_str_map["header"]);
        for (std::string line; std::getline(header, line);)
        {
            if (line.rfind("#include ", 0) == 0)
            {
                lines.push_back(line.substr(9));
            }
        }
        return lines;
    }
};

TEST_F(ProcessorDefaultsTest, SpecNeedsNoIncludes)
{
    ASSERT_TRUE(run(spec()));
    ASSERT_EQ(includes(), (std::vector<std::string>{ "\"streamable.h\"", "<stdint.h>" }));
}

TEST_F(ProcessorDefaultsTest, SpecIncludesFollowLangIncludes)
{
    ASSERT_TRUE(run(spec(R"(["<stdint.h>", "\"extra.h\""])")));
    // The lang file's come first, and one the spec repeats appears once.
    ASSERT_EQ(includes(), (std::vector<std::string>{ "\"streamable.h\"", "<stdint.h>", "\"extra.h\"" }));
}

TEST_F(ProcessorDefaultsTest, LangWithoutIncludesUsesSpecs)
{
    ASSERT_TRUE(run(spec(R"(["\"extra.h\""])"), lang("")));
    ASSERT_EQ(includes(), (std::vector<std::string>{ "\"extra.h\"" }));
}

TEST_F(ProcessorDefaultsTest, LangIncludesAreValidated)
{
    ASSERT_FALSE(run(spec(), lang(R"(["\"a.h\"\n#define x"])")));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
    ASSERT_TRUE(output_str_map.empty());
}

TEST_F(ProcessorDefaultsTest, NamespaceFromSpec)
{
    ASSERT_TRUE(run(spec()));
    ASSERT_NE(output_str_map["header"].find("namespace testspace {"), std::string::npos);
}

TEST_F(ProcessorDefaultsTest, NamespaceOverridesSpec)
{
    name_space = "other";
    ASSERT_TRUE(run(spec()));
    ASSERT_NE(output_str_map["header"].find("namespace other {"), std::string::npos);
    ASSERT_EQ(output_str_map["header"].find("testspace"), std::string::npos);
}

TEST_F(ProcessorDefaultsTest, NamespaceWithoutSpecNamespace)
{
    name_space = "other";
    ASSERT_TRUE(run(spec("", "")));
    ASSERT_NE(output_str_map["header"].find("namespace other {"), std::string::npos);
}

TEST_F(ProcessorDefaultsTest, NamespaceIsValidated)
{
    name_space = "other { evil";
    ASSERT_FALSE(run(spec()));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
    ASSERT_TRUE(output_str_map.empty());
}

TEST_F(ProcessorDefaultsTest, LibraryPassesNamespace)
{
    ASSERT_TRUE(skyloupe::process_string(spec(), output_str_map, lang(), "c++", outputdir, errorcode,
                                         generate_tests, write_to_file, generate_relay, "other"));
    ASSERT_NE(output_str_map["header"].find("namespace other {"), std::string::npos);
}

// proto3 output: the .proto, plus the text-format test message with --generate_tests.
TEST_F(ProcessorUnitTest, Proto3WithTests)
{
    std::ifstream lang_file(std::string(SKYWIRE_LANG_DIR) + "/proto/lang.proto3.json");
    std::stringstream lang;
    lang << lang_file.rdbuf();

    const std::string msg = R"({"stringValues":{"classname":"TestClass","namespace":"testspace"},)"
                            R"("members":{"1":{"type":"int32_t","name":"a"},"2":{"type":"string","name":"s","length":4}}})";

    Processor p;
    ASSERT_TRUE(p.process_string(msg, lang.str(), output_str_map, outputdir, errorcode, true, false));
    ASSERT_EQ(output_str_map.size(), 2u);
    ASSERT_NE(output_str_map["implementation"].find("message TestClass {"), std::string::npos);
    ASSERT_NE(output_str_map["unittest"].find("# expected: "), std::string::npos);
}

// A relay reads each nested message's spec from <type>.json beside the spec,
// and checks the members inside it.
class ProcessorNestedRelayTest : public ::testing::Test
{
protected:
    std::filesystem::path dir;
    results_t output_str_map;
    int errorcode = 0;

    void SetUp() override
    {
        dir = std::filesystem::temp_directory_path() /
              ("skywire_nested_relay_" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::filesystem::remove_all(dir);
        std::filesystem::create_directories(dir);
    }

    void TearDown() override
    {
        std::filesystem::remove_all(dir);
    }

    void write_spec(const std::string& classname, const std::string& members)
    {
        std::ofstream(dir / (classname + ".json"))
            << R"({"stringValues":{"classname":")" << classname << R"(","namespace":"testspace"},"members":)" << members << "}";
    }

    bool run(const std::string& classname, bool generate_relay = true)
    {
        Processor p;
        return p.process_file((dir / (classname + ".json")).string(), std::string(SKYWIRE_LANG_DIR) + "/cpp/lang.cpp.json",
                              output_str_map, dir.string(), errorcode, false, false, generate_relay);
    }
};

TEST_F(ProcessorNestedRelayTest, ChecksMembersInsideNestedMessages)
{
    write_spec("Outer", R"({"1":{"type":"uint8_t","name":"a"},"2":{"type":"Middle","name":"mid"}})");
    write_spec("Middle", R"({"1":{"type":"Inner","name":"in"},"2":{"type":"string","name":"s","length":4}})");
    write_spec("Inner", R"({"1":{"type":"int32_t","name":"x"}})");

    ASSERT_TRUE(run("Outer"));
    const std::string& relay = output_str_map["relay"];
    EXPECT_NE(relay.find("testspace::Inner make_expected_Inner()"), std::string::npos);
    EXPECT_LT(relay.find("make_expected_Inner()"), relay.find("testspace::Middle make_expected_Middle()"));
    EXPECT_NE(relay.find("check_member(\"mid.in.x\", got.Get_mid().Get_in().Get_x(), want.Get_mid().Get_in().Get_x());"), std::string::npos);
    EXPECT_NE(relay.find("check_bytes(\"mid.s\", got.Get_mid().Get_s(), want.Get_mid().Get_s(), 4);"), std::string::npos);
    EXPECT_NE(relay.find("3 members intact"), std::string::npos);
}

TEST_F(ProcessorNestedRelayTest, OnlyRelayNeedsNestedSpec)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"in"}})");
    ASSERT_TRUE(run("Outer", false));
    ASSERT_FALSE(run("Outer"));
    ASSERT_NE(errorcode & SW_FILEREADERROR, 0u);
}

TEST_F(ProcessorNestedRelayTest, NestedSpecMustNameItsType)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"in"}})");
    std::ofstream(dir / "Inner.json") << R"({"stringValues":{"classname":"Other"},"members":{"1":{"type":"int32_t","name":"x"}}})";
    ASSERT_FALSE(run("Outer"));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
}

TEST_F(ProcessorNestedRelayTest, MessageCannotContainItself)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"in"}})");
    write_spec("Inner", R"({"1":{"type":"Outer","name":"out"}})");
    ASSERT_FALSE(run("Outer"));
    ASSERT_NE(errorcode & SW_SPEC_INVALID, 0u);
}

// Two members of the same nested type load its spec once; the relay checks
// both and defines make_expected_Inner() once.
TEST_F(ProcessorNestedRelayTest, NestedTypeUsedTwice)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"a"},"2":{"type":"Inner","name":"b"}})");
    write_spec("Inner", R"({"1":{"type":"int32_t","name":"x"}})");

    ASSERT_TRUE(run("Outer"));
    const std::string& relay = output_str_map["relay"];
    const std::string make = "testspace::Inner make_expected_Inner()";
    ASSERT_NE(relay.find(make), std::string::npos);
    EXPECT_EQ(relay.find(make, relay.find(make) + 1), std::string::npos);
    EXPECT_NE(relay.find("check_member(\"a.x\""), std::string::npos);
    EXPECT_NE(relay.find("check_member(\"b.x\""), std::string::npos);
}

TEST_F(ProcessorNestedRelayTest, UnreadableSpecFile)
{
    ASSERT_FALSE(run("Missing", false));
    ASSERT_EQ(errorcode, SW_FILEREADERROR);
    ASSERT_TRUE(output_str_map.empty());
}

TEST_F(ProcessorNestedRelayTest, UnreadableLangFile)
{
    write_spec("Outer", R"({"1":{"type":"uint8_t","name":"a"}})");

    Processor p;
    ASSERT_FALSE(p.process_file((dir / "Outer.json").string(), (dir / "no_such_lang.json").string(),
        output_str_map, dir.string(), errorcode, false, false));
    ASSERT_EQ(errorcode, SW_FILEREADERROR);
    ASSERT_TRUE(output_str_map.empty());
}

// With the lang file given as a string, a spec file still finds its nested
// specs beside it, and a bad lang string is a string read error.
TEST_F(ProcessorNestedRelayTest, FileWithLangString)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"in"}})");
    write_spec("Inner", R"({"1":{"type":"int32_t","name":"x"}})");

    std::ifstream lang_file(std::string(SKYWIRE_LANG_DIR) + "/cpp/lang.cpp.json");
    std::stringstream lang;
    lang << lang_file.rdbuf();

    Processor p;
    ASSERT_TRUE(p.process_file_langstr((dir / "Outer.json").string(), lang.str(),
        output_str_map, dir.string(), errorcode, false, false, true));
    EXPECT_NE(output_str_map["relay"].find("check_member(\"in.x\""), std::string::npos);

    output_str_map.clear();
    Processor bad;
    ASSERT_FALSE(bad.process_file_langstr((dir / "Outer.json").string(), "not json",
        output_str_map, dir.string(), errorcode, false, false));
    ASSERT_EQ(errorcode, SW_STRINGREADERROR);
}

// A spec named without a directory finds its nested specs in the current one.
TEST_F(ProcessorNestedRelayTest, SpecInCurrentDirectory)
{
    write_spec("Outer", R"({"1":{"type":"Inner","name":"in"}})");
    write_spec("Inner", R"({"1":{"type":"int32_t","name":"x"}})");

    const std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::current_path(dir);
    Processor p;
    const bool ok = p.process_file("Outer.json", std::string(SKYWIRE_LANG_DIR) + "/cpp/lang.cpp.json",
        output_str_map, dir.string(), errorcode, false, false, true);
    std::filesystem::current_path(cwd);

    ASSERT_TRUE(ok);
    EXPECT_NE(output_str_map["relay"].find("check_member(\"in.x\""), std::string::npos);
}

// write_to_file into a directory that doesn't exist reports the write error.
TEST_F(ProcessorNestedRelayTest, UnwritableOutputDirectory)
{
    write_spec("Outer", R"({"1":{"type":"uint8_t","name":"a"}})");

    Processor p;
    ASSERT_FALSE(p.process_file((dir / "Outer.json").string(), std::string(SKYWIRE_LANG_DIR) + "/cpp/lang.cpp.json",
        output_str_map, (dir / "no_such_dir").string(), errorcode, false, true));
    ASSERT_NE(errorcode & SW_FILEWRITEERROR, 0u);
}

// --generate_json, --generate_xml and --generate_bson: each adds its own
// methods (and generated tests), and none changes the output when not given.
namespace
{

std::string read_lang(const std::string& relative)
{
    std::ifstream in(std::string(SKYWIRE_LANG_DIR) + "/" + relative);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

const char* const text_spec = R"({"stringValues":{"classname":"Texty","namespace":"t"},
    "members":{"1":{"type":"int16_t","name":"a"},"2":{"type":"string","name":"s","length":4}}})";

bool generate_text(const std::string& spec, const std::string& lang, bool json, bool xml, results_t& out, int& errorcode,
                   bool bson = false)
{
    Processor p;
    p.set_generate_json(json);
    p.set_generate_xml(xml);
    p.set_generate_bson(bson);
    return p.process_string(spec, read_lang(lang), out, "", errorcode, true, false);
}

bool has(const std::string& text, const std::string& what)
{
    return text.find(what) != std::string::npos;
}

}

TEST(ProcessorTextFormatsTest, OffByDefault)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "cpp/lang.cpp.json", false, false, out, errorcode));
    EXPECT_FALSE(has(out["header"], "text.h"));
    EXPECT_FALSE(has(out["header"], "_json"));
    EXPECT_FALSE(has(out["header"], "_xml"));
    EXPECT_FALSE(has(out["unittest"], "Json"));
    EXPECT_FALSE(has(out["unittest"], "Xml"));
    EXPECT_FALSE(has(out["header"], "bson"));
    EXPECT_FALSE(has(out["unittest"], "Bson"));
}

TEST(ProcessorTextFormatsTest, JsonOnly)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "cpp/lang.cpp.json", true, false, out, errorcode));
    EXPECT_TRUE(has(out["header"], "#include \"text.h\""));
    EXPECT_TRUE(has(out["header"], "std::string to_json() const"));
    EXPECT_TRUE(has(out["header"], "bool from_json(std::string_view text)"));
    EXPECT_FALSE(has(out["header"], "_xml"));
    EXPECT_TRUE(has(out["unittest"], R"(R"sw({"a":-12345,"s":"skyw"})sw")"));
    EXPECT_FALSE(has(out["unittest"], "Xml"));
}

TEST(ProcessorTextFormatsTest, XmlOnly)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "cpp/lang.cpp.json", false, true, out, errorcode));
    EXPECT_TRUE(has(out["header"], "#include \"text.h\""));
    EXPECT_TRUE(has(out["header"], "std::string to_xml() const"));
    EXPECT_TRUE(has(out["header"], "bool from_xml(std::string_view text)"));
    EXPECT_FALSE(has(out["header"], "_json"));
    EXPECT_TRUE(has(out["unittest"], "R\"sw(<Texty><a>-12345</a><s>skyw</s></Texty>)sw\""));
    EXPECT_FALSE(has(out["unittest"], "Json"));
}

TEST(ProcessorTextFormatsTest, BsonOnly)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "cpp/lang.cpp.json", false, false, out, errorcode, true));
    EXPECT_TRUE(has(out["header"], "#include \"bson.h\""));
    EXPECT_FALSE(has(out["header"], "#include \"text.h\""));
    EXPECT_TRUE(has(out["header"], "std::string to_bson() const"));
    EXPECT_TRUE(has(out["header"], "bool from_bson(std::string_view bytes)"));
    EXPECT_FALSE(has(out["header"], "_json"));
    EXPECT_FALSE(has(out["header"], "_xml"));
    // {"a": int32 -12345, "s": string "skyw"}, 24 bytes.
    EXPECT_TRUE(has(out["unittest"], "0x18, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0xc7,\n"
                                     "        0xcf, 0xff, 0xff, 0x02, 0x73, 0x00, 0x05, 0x00,\n"
                                     "        0x00, 0x00, 0x73, 0x6b, 0x79, 0x77, 0x00, 0x00,"));
    EXPECT_FALSE(has(out["unittest"], "Json"));
    EXPECT_FALSE(has(out["unittest"], "Xml"));
}

TEST(ProcessorTextFormatsTest, MemberNameClash)
{
    results_t out;
    int errorcode = 0;
    const char* spec = R"({"stringValues":{"classname":"Texty","namespace":"t"},"members":{"1":{"type":"int16_t","name":"to_xml"}}})";
    EXPECT_TRUE(generate_text(spec, "cpp/lang.cpp.json", true, false, out, errorcode));
    out.clear();
    errorcode = 0;
    EXPECT_FALSE(generate_text(spec, "cpp/lang.cpp.json", false, true, out, errorcode));
    EXPECT_NE(errorcode & SW_SPEC_INVALID, 0u);

    const char* bson_spec = R"({"stringValues":{"classname":"Texty","namespace":"t"},"members":{"1":{"type":"int16_t","name":"from_bson"}}})";
    out.clear();
    errorcode = 0;
    EXPECT_TRUE(generate_text(bson_spec, "cpp/lang.cpp.json", true, true, out, errorcode));
    out.clear();
    errorcode = 0;
    EXPECT_FALSE(generate_text(bson_spec, "cpp/lang.cpp.json", false, false, out, errorcode, true));
    EXPECT_NE(errorcode & SW_SPEC_INVALID, 0u);
}

// C: the same three forms as functions, each only when asked for.
TEST(ProcessorTextFormatsTest, COffByDefault)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "c/lang.c.json", false, false, out, errorcode));
    EXPECT_FALSE(has(out["header"], "ctext.h"));
    EXPECT_FALSE(has(out["header"], "cbson.h"));
    EXPECT_FALSE(has(out["header"], "Json"));
    EXPECT_FALSE(has(out["header"], "Xml"));
    EXPECT_FALSE(has(out["header"], "Bson"));
    EXPECT_FALSE(has(out["unittest"], "Json"));
}

TEST(ProcessorTextFormatsTest, CEachForm)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "c/lang.c.json", true, false, out, errorcode));
    EXPECT_TRUE(has(out["header"], "#include \"ctext.h\""));
    EXPECT_TRUE(has(out["header"], "size_t Texty_ToJson(const Texty * structure, char * buf, size_t cap);"));
    EXPECT_TRUE(has(out["header"], "int Texty_FromJson(Texty * structure, const char * text, size_t len);"));
    EXPECT_TRUE(has(out["implementation"], "sw_json_put_int16_t(w, structure->a);"));
    EXPECT_TRUE(has(out["implementation"], "sw_json_read_bytes(p, decoded.s, 4)"));
    EXPECT_FALSE(has(out["header"], "Xml"));
    EXPECT_FALSE(has(out["header"], "cbson.h"));
    EXPECT_TRUE(has(out["unittest"], R"(R"sw({"a":-12345,"s":"skyw"})sw")"));

    out.clear();
    ASSERT_TRUE(generate_text(text_spec, "c/lang.c.json", false, true, out, errorcode));
    EXPECT_TRUE(has(out["header"], "int Texty_FromXml(Texty * structure, const char * text, size_t len);"));
    EXPECT_TRUE(has(out["implementation"], "sw_xml_is(&p, &root, \"Texty\")"));
    EXPECT_FALSE(has(out["header"], "Json"));
    EXPECT_TRUE(has(out["unittest"], "R\"sw(<Texty><a>-12345</a><s>skyw</s></Texty>)sw\""));

    out.clear();
    ASSERT_TRUE(generate_text(text_spec, "c/lang.c.json", false, false, out, errorcode, true));
    EXPECT_TRUE(has(out["header"], "#include \"cbson.h\""));
    EXPECT_FALSE(has(out["header"], "#include \"ctext.h\""));
    EXPECT_TRUE(has(out["header"], "size_t Texty_ToBson(const Texty * structure, uint8_t * buf, size_t cap);"));
    EXPECT_TRUE(has(out["implementation"], "sw_bson_put_int16_t(o, \"a\", structure->a);"));
    EXPECT_TRUE(has(out["unittest"], "0x18, 0x00, 0x00, 0x00, 0x10, 0x61, 0x00, 0xc7,"));
}

TEST(ProcessorTextFormatsTest, CNestedAndBitfields)
{
    results_t out;
    int errorcode = 0;
    const char* spec = R"({"stringValues":{"classname":"Outer","namespace":"t"},"members":{
        "1":{"type":"Inner","name":"inner"},"2":{"type":"uint8_t","name":"u","bits":3},
        "3":{"type":"unsigned char","name":"c"}}})";
    ASSERT_TRUE(generate_text(spec, "c/lang.c.json", true, true, out, errorcode, true));
    EXPECT_TRUE(has(out["implementation"], "Inner_WriteJson(&structure->inner, w);"));
    EXPECT_TRUE(has(out["implementation"], "Inner_ReadXml(&decoded.inner, p, &child)"));
    EXPECT_TRUE(has(out["implementation"], "e.type != SW_BSON_DOCUMENT || 0 != Inner_ReadBson(&decoded.inner, &e.doc)"));
    EXPECT_TRUE(has(out["implementation"], "sw_json_read_uint8_t(p, &decoded.u, 3)"));
    EXPECT_TRUE(has(out["implementation"], "sw_bson_put_uint8_t(o, \"u\", structure->u);"));
    EXPECT_TRUE(has(out["implementation"], "sw_xml_read_unsigned_char(p, &child, &decoded.c, 0)"));
    EXPECT_TRUE(has(out["implementation"], "sw_bson_put_unsigned_char(o, \"c\", structure->c);"));
}

// Python: methods on the class, and skywire_text.py written beside it.
TEST(ProcessorTextFormatsTest, PythonForms)
{
    results_t out;
    int errorcode = 0;
    ASSERT_TRUE(generate_text(text_spec, "python/lang.python3.json", false, false, out, errorcode));
    EXPECT_EQ(0u, out.count("runtime"));
    EXPECT_FALSE(has(out["implementation"], "skywire_text"));

    out.clear();
    ASSERT_TRUE(generate_text(text_spec, "python/lang.python3.json", true, false, out, errorcode, true));
    ASSERT_EQ(1u, out.count("runtime"));
    EXPECT_TRUE(has(out["runtime"], "class JsonWriter"));
    EXPECT_TRUE(has(out["implementation"], "import skywire_text"));
    EXPECT_TRUE(has(out["implementation"], "    def to_json(self):"));
    EXPECT_TRUE(has(out["implementation"], "            self.a = skywire_text.json_number(field, \"int16_t\")"));
    EXPECT_TRUE(has(out["implementation"], "    def from_bson(self, data):"));
    EXPECT_FALSE(has(out["implementation"], "to_xml"));
    EXPECT_TRUE(has(out["unittest"], "self.assertEqual(text, \"{\\\"a\\\":-12345,\\\"s\\\":\\\"skyw\\\"}\")"));

    const char* clash = R"({"stringValues":{"classname":"Texty","namespace":"t"},"members":{"1":{"type":"int16_t","name":"_read_bson"}}})";
    out.clear();
    errorcode = 0;
    EXPECT_TRUE(generate_text(clash, "python/lang.python3.json", true, true, out, errorcode));
    out.clear();
    EXPECT_FALSE(generate_text(clash, "python/lang.python3.json", false, false, out, errorcode, true));
    EXPECT_NE(errorcode & SW_SPEC_INVALID, 0u);
}

// Other languages refuse the flags until they have the text forms.
TEST(ProcessorTextFormatsTest, OtherLanguagesRefuse)
{
    results_t out;
    int errorcode = 0;
    EXPECT_FALSE(generate_text(text_spec, "go/lang.go.json", true, false, out, errorcode));
    EXPECT_NE(errorcode & SW_ERROR, 0u);
    errorcode = 0;
    EXPECT_FALSE(generate_text(text_spec, "swift/lang.swift.json", false, true, out, errorcode));
    errorcode = 0;
    EXPECT_FALSE(generate_text(text_spec, "rust/lang.rust.json", false, false, out, errorcode, true));
    EXPECT_NE(errorcode & SW_ERROR, 0u);
    errorcode = 0;
    EXPECT_TRUE(generate_text(text_spec, "go/lang.go.json", false, false, out, errorcode));
}
