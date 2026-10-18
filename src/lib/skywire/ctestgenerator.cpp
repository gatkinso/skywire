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

#include <regex>
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
        << "#include <cstring>" << std::endl
        << "#include \"";
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

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        const std::string& name = std::get<member_name>(member.second);

        if (is_string_type(type))
        {
            ss_ << "    strncpy(in." << name << ", \"" << test_values::string_value() << "\", "
                << std::get<member_length>(member.second) << ");\n";
        }
        else
        {
            ss_ << "    in." << name << " = " << test_literal(member.second) << ";\n";
        }
    }

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

}
