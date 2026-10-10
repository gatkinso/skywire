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

    int i = 0;
    for (auto& member : ordered_members_)
    {
        if (is_string_type(std::get<member_type>(member.second)))
        {
            ss_ << std::get<member_length>(member.second);
        }
        else
        {
            ss_ << "sizeof(" << std::get<member_type>(member.second) << ")";
        }
        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << "+";
        }
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
            ss_ << "    in.Set_" << name << "(" << test_values::literal(type) << ");\n";
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
    for (auto& member : ordered_members_)
    {
        if (!test_values::encode(std::get<member_type>(member.second), std::get<member_endianness>(member.second),
                                 std::get<member_length>(member.second), wire))
        {
            // A user-defined member has no shared test value.
            return true;
        }
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

}
