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
#include "cpptestgenerator.h"
#include "test_values.h"

namespace skyloupe
{

bool CppTestGenerator::generate(int& errorcode)
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

bool CppTestGenerator::do_test_implementation(int& errorcode)
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
    test_pz_round_trip(errorcode);
    test_pz_wire(errorcode);
    test_pz_empty(errorcode);
    test_pz_short_buffer(errorcode);
    if (wants_json())
        test_text("json", errorcode);
    if (wants_xml())
        test_text("xml", errorcode);
    if (wants_bson())
        test_bson(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CppTestGenerator::test_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool CppTestGenerator::test_includes(int& errorcode)
{
    std::string hdr_name = msg_spec_.get_value_string("outputheaderfilename");

    ss_ << "#include <gmock/gmock.h>" << std::endl
        << "#include <gtest/gtest.h>" << std::endl
        << "#include <cstdint>" << std::endl
        << "#include <cstring>" << std::endl
        << "#include \"";
    ss_ << hdr_name << "\"" << std::endl << std::endl;

    return true;
}

bool CppTestGenerator::test_serdes(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", SerDes)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in, out;\n";
    ss_ << "    skyloupe::Stream stream;\n\n";
    
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    bool success = in.serialize(stream);\n";
    ss_ << "    ASSERT_TRUE(success);\n\n";
    ss_ << "    success = out.deserialize(stream);\n";
    ss_ << "    ASSERT_TRUE(success);\n";
    ss_ << "}\n\n";

    return true;
}

bool CppTestGenerator::test_size(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", Size)\n";
    ss_ << "{\n";
    insert_indents(1);
    ss_ << namespac << "::" << classname << " msg;\n\n";
    insert_indents(1);
    ss_ << "ASSERT_EQ(msg.size(), ";

    std::vector<std::string> terms;
    for (auto& member : ordered_members_)
    {
        if (is_bitfield(member.second))
        {
            if (place_of(member.first).first)
                terms.push_back("sizeof(" + bitfields::unsigned_type(unit_of(member.first).type) + ")");
        }
        else if (is_string_type(std::get<member_type>(member.second)))
        {
            terms.push_back(std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_user_type(std::get<member_type>(member.second)))
        {
            terms.push_back(namespac + "::" + std::get<member_type>(member.second) + "::size()");
        }
        else
        {
            terms.push_back("sizeof(" + std::get<member_type>(member.second) + ")");
        }
    }
    for (size_t i = 0; i < terms.size(); i++)
    {
        ss_ << terms[i] << (i + 1 < terms.size() ? "+" : "");
    }

    ss_ << ");\n";
    ss_ << "}\n\n";

    return true;
}

bool CppTestGenerator::test_equality(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", EqualityOperator)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in, out;\n";
    ss_ << "    skyloupe::Stream stream;\n\n";

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    bool success = in.serialize(stream);\n";
    ss_ << "    ASSERT_TRUE(success);\n\n";
    ss_ << "    success = out.deserialize(stream);\n";
    ss_ << "    ASSERT_TRUE(success);\n\n";
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "}\n\n";

    return true;
}

namespace
{

// text as a C++ raw string literal.
std::string raw(const std::string& text)
{
    return "R\"sw(" + text + ")sw\"";
}

}

// The text form's tests: the exact text (the same in every language, from
// test_values), a round trip, missing members reading as zero, and malformed
// or out-of-range input failing without changing the message.
bool CppTestGenerator::test_text(const std::string& format, int& errorcode)
{
    const std::string namespac = msg_spec_.get_value_string("namespace");
    const std::string classname = msg_spec_.get_value_string("classname");
    const std::string qualified = namespac + "::" + classname;
    const std::string testname = classname + "Test";
    const bool json = format == "json";
    const std::string Format = json ? "Json" : "Xml";
    const std::string to = "to_" + format + "()";
    const std::string from = "from_" + format;

    std::string expected;
    const bool have_expected = json ? expected_json(expected) : expected_xml(expected);

    ss_ << "TEST(" << testname << ", " << Format << ")\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    const std::string text = in." << to << ";\n";
    if (have_expected)
    {
        ss_ << "    ASSERT_EQ(" << raw(expected) << ", text);\n";
    }
    ss_ << "    ASSERT_TRUE(out." << from << "(text));\n";
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "    ASSERT_EQ(text, out." << to << ");\n";
    ss_ << "}\n\n";

    // An empty object or element is the all-zero message.
    ss_ << "TEST(" << testname << ", " << Format << "EmptyIsDefault)\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    out = in;\n";
    ss_ << "    ASSERT_TRUE(out." << from << "(" << raw(json ? "{}" : "<" + classname + "/>") << "));\n";
    ss_ << "    ASSERT_EQ(" << qualified << "(), out);\n";
    ss_ << "}\n\n";

    // Each of these fails, leaving the message as it was.
    const std::vector<std::string> bad = json ? rejected_json() : rejected_xml();

    ss_ << "TEST(" << testname << ", " << Format << "Rejects)\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    out = in;\n";
    for (const auto& text : bad)
    {
        ss_ << "    EXPECT_FALSE(out." << from << "(" << raw(text) << ")) << " << raw(text) << ";\n";
    }
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "}\n\n";

    return true;
}

// The BSON tests: the exact bytes (the same in every language, from
// test_values), a round trip, an empty document reading as zero, uint64_t
// values above 2^63-1 as Decimal128, and malformed or out-of-range documents
// failing without changing the message.
bool CppTestGenerator::test_bson(int& errorcode)
{
    const std::string namespac = msg_spec_.get_value_string("namespace");
    const std::string classname = msg_spec_.get_value_string("classname");
    const std::string qualified = namespac + "::" + classname;
    const std::string testname = classname + "Test";

    std::vector<uint8_t> expected;
    const bool have_expected = expected_bson(expected);

    ss_ << "TEST(" << testname << ", Bson)\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    const std::string bytes = in.to_bson();\n";
    if (have_expected)
    {
        ss_ << "    static const uint8_t want[] = {" << byte_list(expected, "        ") << "\n    };\n";
        ss_ << "    ASSERT_EQ(std::string(reinterpret_cast<const char*>(want), sizeof(want)), bytes);\n";
    }
    ss_ << "    ASSERT_TRUE(out.from_bson(bytes));\n";
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "    ASSERT_EQ(bytes, out.to_bson());\n";
    ss_ << "}\n\n";

    ss_ << "TEST(" << testname << ", BsonEmptyIsDefault)\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    out = in;\n";
    ss_ << "    ASSERT_TRUE(out.from_bson(std::string(\"\\x05\\0\\0\\0\\0\", 5)));\n";
    ss_ << "    ASSERT_EQ(" << qualified << "(), out);\n";
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
        ss_ << "    " << qualified << " in, out;\n";
        ss_ << "    static const uint8_t big[] = {" << byte_list(big, "        ") << "\n    };\n";
        ss_ << "    static const uint8_t edge[] = {" << byte_list(edge, "        ") << "\n    };\n";
        ss_ << "    in.Set_" << name << "(UINT64_C(18446744073709551615));\n";
        ss_ << "    std::string bytes = in.to_bson();\n";
        ss_ << "    ASSERT_NE(std::string::npos, bytes.find(std::string(reinterpret_cast<const char*>(big), sizeof(big))));\n";
        ss_ << "    ASSERT_TRUE(out.from_bson(bytes));\n";
        ss_ << "    ASSERT_EQ(in, out);\n";
        ss_ << "    in.Set_" << name << "(UINT64_C(9223372036854775807));\n";
        ss_ << "    bytes = in.to_bson();\n";
        ss_ << "    ASSERT_NE(std::string::npos, bytes.find(std::string(reinterpret_cast<const char*>(edge), sizeof(edge))));\n";
        ss_ << "    ASSERT_TRUE(out.from_bson(bytes));\n";
        ss_ << "    ASSERT_EQ(in, out);\n";
        ss_ << "}\n\n";
    }

    // Documents that fail, leaving the message as it was.
    const std::vector<std::vector<uint8_t>> bad = rejected_bson();

    ss_ << "TEST(" << testname << ", BsonRejects)\n";
    ss_ << "{\n";
    ss_ << "    " << qualified << " in, out;\n";
    do_sets(errorcode);
    ss_ << "\n";
    ss_ << "    out = in;\n";
    ss_ << "    const std::string good = in.to_bson();\n";
    ss_ << "    std::string longer = good;\n";
    ss_ << "    longer[0] = static_cast<char>(longer[0] + 1);\n";
    ss_ << "    EXPECT_FALSE(out.from_bson(good.substr(0, good.size() - 1)));\n";
    ss_ << "    EXPECT_FALSE(out.from_bson(good + '\\0'));\n";
    ss_ << "    EXPECT_FALSE(out.from_bson(longer));\n";
    ss_ << "    static const std::vector<std::vector<uint8_t>> bad = {\n";
    for (const auto& doc : bad)
    {
        ss_ << "        {" << byte_list(doc, "            ") << (doc.empty() ? "},\n" : "\n        },\n");
    }
    ss_ << "    };\n";
    ss_ << "    for (size_t i = 0; i < bad.size(); i++)\n";
    ss_ << "    {\n";
    ss_ << "        EXPECT_FALSE(out.from_bson(std::string(bad[i].begin(), bad[i].end()))) << \"document \" << i;\n";
    ss_ << "    }\n";
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "}\n\n";

    return true;
}

// Every member gets the value shared by all the test generators.
bool CppTestGenerator::do_sets(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);

        if (is_string_type(type))
        {
            ss_ << "    in.Set_" << name << "(\"" << test_values::string_value() << "\");\n";
        }
        else if (test_values::literal(type))
        {
            ss_ << "    in.Set_" << name << "(" << test_literal(member.second) << ");\n";
        }
    }

    return true;
}

bool CppTestGenerator::test_short_buffer(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    // One byte short of a full message: deserialize must fail, consume
    // nothing, and leave the destination untouched.
    ss_ << "TEST(" << testname << ", ShortBuffer)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in;\n";

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    skyloupe::Stream full;\n";
    ss_ << "    ASSERT_TRUE(in.serialize(full));\n";
    ss_ << "    ASSERT_EQ(full.byte_size(), in.size());\n\n";
    ss_ << "    skyloupe::Stream stream;\n";
    ss_ << "    stream.write(full.data(), full.byte_size() - 1);\n\n";
    ss_ << "    " << namespac << "::" << classname << " out = in;\n";
    ss_ << "    ASSERT_FALSE(out.deserialize(stream));\n";
    ss_ << "    ASSERT_EQ(out, in);\n";
    ss_ << "    ASSERT_EQ(stream.byte_size(), in.size() - 1);\n";
    ss_ << "}\n\n";

    return true;
}

// Compares the serialized bytes with the ones computed by wiregen, so every
// language is checked against the same expected wire bytes.
bool CppTestGenerator::test_wire(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
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
    ss_ << "    " << namespac << "::" << classname << " in;\n";

    do_sets(errorcode);

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
    ss_ << "    skyloupe::Stream stream;\n";
    ss_ << "    ASSERT_TRUE(in.serialize(stream));\n";
    ss_ << "    ASSERT_EQ(sizeof(want), stream.byte_size());\n";
    ss_ << "    ASSERT_EQ(0, std::memcmp(stream.data(), want, sizeof(want)));\n";
    ss_ << "}\n\n";

    return true;
}

// A bitfield setter keeps only the field's bits: a value with a bit set
// above the field reads back as the field's value.
bool CppTestGenerator::test_bitfield_setters(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
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
        body << "    msg.Set_" << name << "(static_cast<" << type << ">(" << wide << "ull));\n";
        body << "    ASSERT_EQ(static_cast<" << type << ">(" << test_literal(member.second) << "), msg.Get_" << name << "());\n";
    }

    if (body.str().empty())
    {
        return true;
    }

    ss_ << "TEST(" << classname << "Test, BitfieldSetters)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " msg;\n";
    ss_ << body.str();
    ss_ << "}\n\n";

    return true;
}

// The PZ (protobuf) tests: the same message through a PZ-mode stream.
bool CppTestGenerator::test_pz_round_trip(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", PZRoundTrip)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in, out;\n";

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);\n";
    ss_ << "    ASSERT_TRUE(in.serialize(stream));\n";
    ss_ << "    ASSERT_TRUE(out.deserialize(stream));\n";
    ss_ << "    ASSERT_EQ(in, out);\n";
    ss_ << "    ASSERT_EQ(0u, stream.byte_size());\n";
    ss_ << "}\n\n";

    return true;
}

// Compares the protobuf bytes with the ones computed by wiregen, which the
// proto3 test (Proto3Wire) also checks protoc produces for the generated .proto.
bool CppTestGenerator::test_pz_wire(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    std::vector<uint8_t> wire;
    for (auto& member : ordered_members_)
    {
        // A nested message is left all zero, so protobuf leaves it out.
        if (is_user_type(std::get<member_type>(member.second)))
        {
            continue;
        }
        if (!test_values::encode_pb(std::get<member_type>(member.second), member.first,
                                    std::get<member_length>(member.second), wire, std::get<member_bits>(member.second)))
        {
            return true;
        }
    }

    ss_ << "TEST(" << testname << ", PZWire)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in;\n";

    do_sets(errorcode);

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
    ss_ << "    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);\n";
    ss_ << "    ASSERT_TRUE(in.serialize(stream));\n";
    ss_ << "    ASSERT_EQ(sizeof(want), stream.byte_size());\n";
    ss_ << "    ASSERT_EQ(0, std::memcmp(stream.data(), want, sizeof(want)));\n";
    ss_ << "}\n\n";

    return true;
}

// proto3 leaves out zero fields, so an empty message is a valid encoding of
// the all-zero message.
bool CppTestGenerator::test_pz_empty(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", PZEmptyIsDefault)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " zero, in, out;\n";

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);\n";
    ss_ << "    ASSERT_TRUE(zero.serialize(stream));\n";
    ss_ << "    ASSERT_EQ(0u, stream.byte_size());\n\n";
    ss_ << "    out = in;\n";
    ss_ << "    ASSERT_TRUE(out.deserialize(stream));\n";
    ss_ << "    ASSERT_EQ(zero, out);\n";
    ss_ << "}\n\n";

    return true;
}

// A message cut one byte short is malformed: deserialize must fail, consume
// nothing, and leave the destination untouched.
bool CppTestGenerator::test_pz_short_buffer(int& errorcode)
{
    std::string namespac = msg_spec_.get_value_string("namespace");
    std::string classname = msg_spec_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", PZShortBuffer)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in;\n";

    do_sets(errorcode);

    ss_ << "\n";
    ss_ << "    skyloupe::Stream full(512, 512, skyloupe::Stream::PZ);\n";
    ss_ << "    ASSERT_TRUE(in.serialize(full));\n";
    ss_ << "    ASSERT_LT(0u, full.byte_size());\n\n";
    ss_ << "    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);\n";
    ss_ << "    stream.pz_buffer() = full.pz_buffer().substr(0, full.byte_size() - 1);\n\n";
    ss_ << "    " << namespac << "::" << classname << " out;\n";
    ss_ << "    ASSERT_FALSE(out.deserialize(stream));\n";
    ss_ << "    ASSERT_EQ(out, " << namespac << "::" << classname << "());\n";
    ss_ << "    ASSERT_EQ(stream.byte_size(), full.byte_size() - 1);\n";
    ss_ << "}\n\n";

    return true;
}

}
