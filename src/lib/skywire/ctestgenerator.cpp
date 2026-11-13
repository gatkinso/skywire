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

#include <map>
#include <cstdio>
#include "ctestgenerator.h"
#include "test_values.h"

namespace skyloupe
{

bool CTestGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    if (false == do_test_implementation(errorcode))
    {
        errorcode |= SW_FILE_HEADER;
        return false;
    }

    return true;
}

bool CTestGenerator::do_test_implementation(int& errorcode)
{
    std::string& header_string = results_["unittest"];

    test_banner(errorcode);
    test_includes(errorcode);
    test_serdes(errorcode);
    test_size(errorcode);
    test_equality(errorcode);
    test_short_buffer(errorcode);
    test_wire(errorcode);
    test_bitfield_setters(errorcode);
    if (wants_json())
        test_text("json", errorcode);
    if (wants_xml())
        test_text("xml", errorcode);
    if (wants_bson())
        test_bson(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CTestGenerator::test_banner(int& errorcode)
{
    return gen_one_line("banner_c_str");
}

bool CTestGenerator::test_includes(int& errorcode)
{
    std::string hdr_name = msg_spec_.get_value_string("outputheaderfilename");

    ss_ << "#include <gmock/gmock.h>" << std::endl
        << "#include <gtest/gtest.h>" << std::endl
        << "#include <cstring>" << std::endl;
    if (wants_json() || wants_xml() || wants_bson())
    {
        ss_ << "#include <algorithm>" << std::endl
            << "#include <string>" << std::endl
            << "#include <vector>" << std::endl;
    }
    ss_ << "#include \"";
    ss_ << hdr_name << "\"" << std::endl << std::endl;

    return true;
}

bool CTestGenerator::test_serdes(int& errorcode)
{
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", SerDes)\n";
    ss_ << "{\n";
    ss_ << "    Cstream cs;\n\n";

    insert_indents(1);
    ss_ << "const int NUM = 1000;\n";
    insert_indents(1);
    ss_ << classname << " out[NUM];\n";
    
    insert_indents(1);
    ss_ << "cstream_init(&cs, 128);\n";
    
    insert_indents(1);
    ss_ << "int sign = 1;\n";
    insert_indents(1);
    ss_ << "for (uint32_t i = 0; i < NUM; i++)\n";
    insert_indents(1);
    ss_ << "{\n";
    
    do_sets(errorcode);
    
    insert_indents(2);
    ss_ << classname << "_Serialize(&out[i], &cs);\n";
    insert_indents(1);
    ss_ << "}\n\n";
    
    insert_indents(1);
    ss_ << "for (uint32_t i = 0; i < NUM; i++)\n";
    insert_indents(1);
    ss_ << "{\n";
    insert_indents(2);
    ss_ << classname << " in;\n";
    insert_indents(2);
    ss_ << classname << "_Deserialize(&in, &cs);\n\n";
    
    insert_indents(2);
    ss_ << "ASSERT_EQ(1, " << classname << "_Equals(&out[i], &in));\n";
    insert_indents(1);
    ss_ << "}\n\n";

    insert_indents(1);
    ss_ << "cstream_fini(&cs);\n";

    ss_ << "}\n\n";

    return true;
}

bool CTestGenerator::test_size(int& errorcode)
{
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", Size)\n";
    ss_ << "{\n";
    insert_indents(1);
    ss_ << "ASSERT_EQ(" << classname << "_Size(),";

    std::vector<std::string> terms;
    for (auto& member : ordered_members_)
    {
        if (is_bitfield(member.second))
        {
            if (place_of(member.first).first)
                terms.push_back("sizeof(" + bitfields::unsigned_type(unit_of(member.first).type) + ")");
        }
        else if (is_string_type(std::get<member_type>(member.second)))
            terms.push_back(std::to_string(std::get<member_length>(member.second)));
        else if (is_user_type(std::get<member_type>(member.second)))
            terms.push_back(std::get<member_type>(member.second) + "_Size()");
        else
            terms.push_back("sizeof(" + std::get<member_type>(member.second) + ")");
    }
    for (size_t i = 0; i < terms.size(); i++)
    {
        ss_ << terms[i] << (i + 1 < terms.size() ? "+" : "");
    }

    ss_ << ");\n";
    ss_ << "}\n\n";

    return true;
}

bool CTestGenerator::test_equality(int& errorcode)
{
    return true;
}

bool CTestGenerator::do_sets(int& errorcode)
{
    int sign = 1;
    int i = 1;
    for (auto& member : ordered_members_)
    {
        insert_indents(2);

        // A bitfield gets a value that fits, so it survives the round trip.
        if (is_bitfield(member.second))                                   ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << test_literal(member.second) << ";\n";
        else if (std::get<member_type>(member.second) == "char")          ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << (int)i++*BASECHAR << ";\n";
        else if (std::get<member_type>(member.second) == "unsigned char") ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "i*" << i++*BASEUCHAR << ";\n";
        else if (std::get<member_type>(member.second) == "uint8_t")       ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "i*" << i++*BASEU8 << ";\n";
        else if (std::get<member_type>(member.second) == "int8_t")        ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASES8 << ";\n";
        else if (std::get<member_type>(member.second) == "uint16_t")      ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "i*" << i++*BASEU16 << ";\n";
        else if (std::get<member_type>(member.second) == "int16_t")       ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASES16 << ";\n";
        else if (std::get<member_type>(member.second) == "uint32_t")      ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "i*" << i++*BASEU32 << ";\n";
        else if (std::get<member_type>(member.second) == "int32_t")       ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASES32 << ";\n";
        else if (std::get<member_type>(member.second) == "uint64_t")      ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "i*" << i++*BASEU64 << ";\n";
        else if (std::get<member_type>(member.second) == "int64_t")       ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASES64 << ";\n";
        else if (std::get<member_type>(member.second) == "float")         ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASEFLOAT << "f;\n";
        else if (std::get<member_type>(member.second) == "double")        ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << "sign*i*" << i++*BASEDOUBLE << ";\n";
        else if (std::get<member_type>(member.second) == "string")        ss_ << "strncpy(out[i]." << std::get<member_name>(member.second) << ", \"skywire\", " << std::get<member_length>(member.second) << ");\n";
        // A nested message has no test value; zero it, so it compares equal after the round trip.
        else if (is_user_type(std::get<member_type>(member.second)))     ss_ << "memset(&out[i]." << std::get<member_name>(member.second) << ", 0, sizeof(out[i]." << std::get<member_name>(member.second) << "));\n";
    }

    insert_indents(2);
    ss_ << "sign *= -1;\n";

    return true;
}

bool CTestGenerator::test_short_buffer(int& errorcode)
{
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    // One byte short of a full message: Deserialize must fail, consume
    // nothing, and leave the destination untouched.
    ss_ << "TEST(" << testname << ", ShortBuffer)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in;\n";
    ss_ << "    std::memset(&in, 0x5a, sizeof(in));\n";
    ss_ << "    Cstream full;\n";
    ss_ << "    cstream_init(&full, 128);\n";
    ss_ << "    ASSERT_EQ(0, " << classname << "_Serialize(&in, &full));\n";
    ss_ << "    ASSERT_EQ(" << classname << "_Size(), cstream_available(&full));\n\n";
    ss_ << "    Cstream cs;\n";
    ss_ << "    cstream_init(&cs, 128);\n";
    ss_ << "    cstream_write(&cs, full.head_, " << classname << "_Size() - 1, SW_NATIVE_ENDIAN_FLAG);\n\n";
    ss_ << "    " << classname << " out;\n";
    ss_ << "    std::memset(&out, 0x11, sizeof(out));\n";
    ss_ << "    " << classname << " before = out;\n\n";
    ss_ << "    ASSERT_EQ(-1, " << classname << "_Deserialize(&out, &cs));\n";
    ss_ << "    ASSERT_EQ(0, std::memcmp(&out, &before, sizeof(out)));\n";
    ss_ << "    ASSERT_EQ(" << classname << "_Size() - 1, cstream_available(&cs));\n\n";
    ss_ << "    cstream_fini(&cs);\n";
    ss_ << "    cstream_fini(&full);\n";
    ss_ << "}\n\n";

    return true;
}

// Sets every member to the value shared by all the test generators and
// compares the serialized bytes with the ones computed by wiregen, so every
// language is checked against the same expected wire bytes.
bool CTestGenerator::test_wire(int& errorcode)
{
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    std::vector<uint8_t> wire;
    if (false == expected_wire(wire))
    {
        // A user-defined member has no shared test value.
        return true;
    }

    ss_ << "TEST(" << testname << ", Wire)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    do_test_value_sets();

    ss_ << "\n";
    ss_ << "    static const uint8_t want[] = {";
    for (size_t i = 0; i < wire.size(); i++)
    {
        ss_ << ((i % 8 == 0) ? "\n        " : " ");
        char hex[8];
        std::snprintf(hex, sizeof(hex), "0x%02x,", wire[i]);
        ss_ << hex;
    }
    ss_ << "\n    };\n\n";
    ss_ << "    Cstream cs;\n";
    ss_ << "    cstream_init(&cs, 128);\n";
    ss_ << "    ASSERT_EQ(0, " << classname << "_Serialize(&in, &cs));\n";
    ss_ << "    ASSERT_EQ(sizeof(want), cstream_available(&cs));\n";
    ss_ << "    ASSERT_EQ(0, std::memcmp(cs.head_, want, sizeof(want)));\n";
    ss_ << "    cstream_fini(&cs);\n";
    ss_ << "}\n\n";

    return true;
}

// A bitfield setter keeps only the field's bits: a value with a bit set
// above the field reads back as the field's value.
bool CTestGenerator::test_bitfield_setters(int& errorcode)
{
    std::string classname = msg_spec_.get_value_string("classname");

    std::stringstream body;
    for (auto& member : ordered_members_)
    {
        const std::string wide = out_of_range_hex(member.second);
        if (wide.empty())
        {
            continue;
        }
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);
        body << "    {\n";
        body << "        " << type << " value = (" << type << ")" << wide << "ull, got = 0;\n";
        body << "        ASSERT_EQ(0, Set_" << classname << "_" << name << "(&msg, &value));\n";
        body << "        ASSERT_EQ(0, Get_" << classname << "_" << name << "(&msg, &got));\n";
        body << "        ASSERT_EQ((" << type << ")" << test_literal(member.second) << ", got);\n";
        body << "    }\n";
    }

    if (body.str().empty())
    {
        return true;
    }

    ss_ << "TEST(" << classname << "Test, BitfieldSetters)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " msg;\n";
    ss_ << "    std::memset(&msg, 0, sizeof(msg));\n";
    ss_ << body.str();
    ss_ << "}\n\n";

    return true;
}

// Gives each member of in the value shared by all the test generators; a
// nested message has none, and stays zero.
void CTestGenerator::do_test_value_sets()
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);

        if (is_string_type(type))
        {
            ss_ << "    strncpy(in." << name << ", \"" << test_values::string_value() << "\", "
                << std::get<member_length>(member.second) << ");\n";
        }
        else if (false == is_user_type(type))
        {
            ss_ << "    in." << name << " = " << test_literal(member.second) << ";\n";
        }
    }
}

namespace
{

// text as a C++ raw string literal.
std::string raw(const std::string& text)
{
    return "R\"sw(" + text + ")sw\"";
}

}

// The JSON or XML tests: the exact text (the same in every language, from
// test_values), the snprintf contract, a round trip, an empty object or
// element reading as zero, and malformed or out-of-range input failing
// without changing the message.
bool CTestGenerator::test_text(const std::string& format, int& errorcode)
{
    const std::string classname = msg_spec_.get_value_string("classname");
    const std::string testname = classname + "Test";
    const bool json = format == "json";
    const std::string Format = json ? "Json" : "Xml";
    const std::string to = classname + "_To" + Format;
    const std::string from = classname + "_From" + Format;

    std::string expected;
    const bool have_expected = json ? expected_json(expected) : expected_xml(expected);

    ss_ << "TEST(" << testname << ", " << Format << ")\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    ss_ << "    std::memset(&out, 0, sizeof(out));\n";
    do_test_value_sets();
    ss_ << "\n";
    ss_ << "    const size_t n = " << to << "(&in, NULL, 0);\n";
    ss_ << "    std::vector<char> buf(n + 1);\n";
    ss_ << "    ASSERT_EQ(n, " << to << "(&in, buf.data(), buf.size()));\n";
    ss_ << "    const std::string text(buf.data(), n);\n";
    if (have_expected)
    {
        ss_ << "    ASSERT_EQ(" << raw(expected) << ", text);\n";
    }
    ss_ << "    char small[8];\n";
    ss_ << "    ASSERT_EQ(n, " << to << "(&in, small, sizeof(small)));\n";
    ss_ << "    ASSERT_EQ(text.substr(0, sizeof(small) - 1), std::string(small));\n";
    ss_ << "    ASSERT_EQ(0, " << from << "(&out, text.data(), text.size()));\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&in, &out));\n";
    ss_ << "    std::vector<char> again(n + 1);\n";
    ss_ << "    ASSERT_EQ(n, " << to << "(&out, again.data(), again.size()));\n";
    ss_ << "    ASSERT_EQ(text, std::string(again.data(), n));\n";
    ss_ << "}\n\n";

    const std::string empty = json ? "{}" : "<" + classname + "/>";
    ss_ << "TEST(" << testname << ", " << Format << "EmptyIsDefault)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out, zero;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    ss_ << "    std::memset(&zero, 0, sizeof(zero));\n";
    do_test_value_sets();
    ss_ << "    out = in;\n";
    ss_ << "    ASSERT_EQ(0, " << from << "(&out, \"" << empty << "\", " << empty.size() << "));\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&zero, &out));\n";
    ss_ << "}\n\n";

    ss_ << "TEST(" << testname << ", " << Format << "Rejects)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    do_test_value_sets();
    ss_ << "    out = in;\n";
    ss_ << "    static const char* const bad[] = {\n";
    for (const auto& text : json ? rejected_json() : rejected_xml())
    {
        ss_ << "        " << raw(text) << ",\n";
    }
    ss_ << "    };\n";
    ss_ << "    for (const char* text : bad)\n";
    ss_ << "    {\n";
    ss_ << "        EXPECT_EQ(-1, " << from << "(&out, text, std::strlen(text))) << text;\n";
    ss_ << "    }\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&in, &out));\n";
    ss_ << "}\n\n";

    return true;
}

// The BSON tests: the exact bytes (the same in every language, from
// test_values), a round trip, an empty document reading as zero, uint64_t
// values above 2^63-1 as Decimal128, and malformed or out-of-range documents
// failing without changing the message.
bool CTestGenerator::test_bson(int& errorcode)
{
    const std::string classname = msg_spec_.get_value_string("classname");
    const std::string testname = classname + "Test";

    std::vector<uint8_t> expected;
    const bool have_expected = expected_bson(expected);

    ss_ << "TEST(" << testname << ", Bson)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    ss_ << "    std::memset(&out, 0, sizeof(out));\n";
    do_test_value_sets();
    ss_ << "\n";
    ss_ << "    const size_t n = " << classname << "_ToBson(&in, NULL, 0);\n";
    ss_ << "    std::vector<uint8_t> bytes(n);\n";
    ss_ << "    ASSERT_EQ(n, " << classname << "_ToBson(&in, bytes.data(), bytes.size()));\n";
    if (have_expected)
    {
        ss_ << "    static const uint8_t want[] = {" << byte_list(expected, "        ") << "\n    };\n";
        ss_ << "    ASSERT_EQ(std::vector<uint8_t>(want, want + sizeof(want)), bytes);\n";
    }
    ss_ << "    ASSERT_EQ(0, " << classname << "_FromBson(&out, bytes.data(), bytes.size()));\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&in, &out));\n";
    ss_ << "    std::vector<uint8_t> again(n);\n";
    ss_ << "    ASSERT_EQ(n, " << classname << "_ToBson(&out, again.data(), again.size()));\n";
    ss_ << "    ASSERT_EQ(bytes, again);\n";
    ss_ << "}\n\n";

    ss_ << "TEST(" << testname << ", BsonEmptyIsDefault)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out, zero;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    ss_ << "    std::memset(&zero, 0, sizeof(zero));\n";
    do_test_value_sets();
    ss_ << "    out = in;\n";
    ss_ << "    static const uint8_t empty[] = { 0x05, 0x00, 0x00, 0x00, 0x00 };\n";
    ss_ << "    ASSERT_EQ(0, " << classname << "_FromBson(&out, empty, sizeof(empty)));\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&zero, &out));\n";
    ss_ << "}\n\n";

    // A uint64_t above 2^63-1 is a Decimal128, so no tool sees it negative;
    // 2^63-1 itself is still an int64.
    for (auto& member : ordered_members_)
    {
        const std::string& name = std::get<member_name>(member.second);
        if (std::get<member_type>(member.second) != "uint64_t" || is_bitfield(member.second))
            continue;

        std::vector<uint8_t> big, edge;
        test_values::bson_decimal128(name, 0, 0xFFFFFFFFFFFFFFFFull, false, big);
        test_values::bson_int64(name, 0x7FFFFFFFFFFFFFFFll, edge);

        ss_ << "TEST(" << testname << ", BsonLargeUint64_" << name << ")\n";
        ss_ << "{\n";
        ss_ << "    " << classname << " in, out;\n";
        ss_ << "    std::memset(&in, 0, sizeof(in));\n";
        ss_ << "    std::memset(&out, 0, sizeof(out));\n";
        ss_ << "    static const uint8_t big[] = {" << byte_list(big, "        ") << "\n    };\n";
        ss_ << "    static const uint8_t edge[] = {" << byte_list(edge, "        ") << "\n    };\n";
        ss_ << "    for (int k = 0; k < 2; k++)\n";
        ss_ << "    {\n";
        ss_ << "        const uint8_t* const element = k == 0 ? big : edge;\n";
        ss_ << "        const size_t element_size = k == 0 ? sizeof(big) : sizeof(edge);\n";
        ss_ << "        in." << name << " = k == 0 ? UINT64_C(18446744073709551615) : UINT64_C(9223372036854775807);\n";
        ss_ << "        std::vector<uint8_t> bytes(" << classname << "_ToBson(&in, NULL, 0));\n";
        ss_ << "        " << classname << "_ToBson(&in, bytes.data(), bytes.size());\n";
        ss_ << "        ASSERT_NE(bytes.end(), std::search(bytes.begin(), bytes.end(), element, element + element_size));\n";
        ss_ << "        ASSERT_EQ(0, " << classname << "_FromBson(&out, bytes.data(), bytes.size()));\n";
        ss_ << "        ASSERT_EQ(1, " << classname << "_Equals(&in, &out));\n";
        ss_ << "    }\n";
        ss_ << "}\n\n";
    }

    ss_ << "TEST(" << testname << ", BsonRejects)\n";
    ss_ << "{\n";
    ss_ << "    " << classname << " in, out;\n";
    ss_ << "    std::memset(&in, 0, sizeof(in));\n";
    do_test_value_sets();
    ss_ << "    out = in;\n";
    ss_ << "    std::vector<uint8_t> good(" << classname << "_ToBson(&in, NULL, 0));\n";
    ss_ << "    " << classname << "_ToBson(&in, good.data(), good.size());\n";
    ss_ << "    EXPECT_EQ(-1, " << classname << "_FromBson(&out, good.data(), good.size() - 1));\n";
    ss_ << "    std::vector<uint8_t> longer = good;\n";
    ss_ << "    longer.push_back(0);\n";
    ss_ << "    EXPECT_EQ(-1, " << classname << "_FromBson(&out, longer.data(), longer.size()));\n";
    ss_ << "    longer = good;\n";
    ss_ << "    longer[0] = static_cast<uint8_t>(longer[0] + 1);\n";
    ss_ << "    EXPECT_EQ(-1, " << classname << "_FromBson(&out, longer.data(), longer.size()));\n";
    ss_ << "    static const std::vector<std::vector<uint8_t>> bad = {\n";
    for (const auto& doc : rejected_bson())
    {
        ss_ << "        {" << byte_list(doc, "            ") << (doc.empty() ? "},\n" : "\n        },\n");
    }
    ss_ << "    };\n";
    ss_ << "    for (size_t i = 0; i < bad.size(); i++)\n";
    ss_ << "    {\n";
    ss_ << "        EXPECT_EQ(-1, " << classname << "_FromBson(&out, bad[i].data(), bad[i].size())) << \"document \" << i;\n";
    ss_ << "    }\n";
    ss_ << "    ASSERT_EQ(1, " << classname << "_Equals(&in, &out));\n";
    ss_ << "}\n\n";

    return true;
}

}
