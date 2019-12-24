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
#include "cpptestgenerator.h"

namespace skyloupe
{

CppTestGenerator::CppTestGenerator(Stencil& stencil) : Generator(stencil)
{

}

bool CppTestGenerator::generate(int& errorcode)
{
    std::cout << "Generating C++ Unit Tests" << std::endl;
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

    header_string.append(ss_.str());

    return true;
}

bool CppTestGenerator::test_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool CppTestGenerator::test_includes(int& errorcode)
{
    std::string hdr_name = msg_stencil_.get_value_string("outputheaderfilename");

    ss_ << "#include <gmock/gmock.h>" << std::endl
        << "#include <gtest/gtest.h>" << std::endl
        << "#include \"";
    ss_ << hdr_name << "\"" << std::endl << std::endl;

    return true;
}

bool CppTestGenerator::test_serdes(int& errorcode)
{
    std::string namespac = msg_stencil_.get_value_string("namespace");
    std::string classname = msg_stencil_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", SerDes)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in, out;\n";
    ss_ << "    skyloupe::Stream stream;\n\n";
    
    if (msg_stencil_.get_proto().members_size() == 0)
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
    std::string namespac = msg_stencil_.get_value_string("namespace");
    std::string classname = msg_stencil_.get_value_string("classname");
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
        ss_ << "sizeof(" << std::get<member_type>(member.second) << ")";
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
    std::string namespac = msg_stencil_.get_value_string("namespace");
    std::string classname = msg_stencil_.get_value_string("classname");
    std::string testname = classname + "Test";

    ss_ << "TEST(" << testname << ", EqualityOperator)\n";
    ss_ << "{\n";
    ss_ << "    " << namespac << "::" << classname << " in, out;\n";
    ss_ << "    skyloupe::Stream stream;\n\n";

    if (msg_stencil_.get_proto().members_size() == 0)
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

bool CppTestGenerator::do_sets(int& errorcode)
{
    int sign = 1;
    int i = 1;
    for (auto& member : ordered_members_)
    {
        insert_indents(1);

             if (std::get<member_type>(member.second) == "char")          ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << (int)i++*BASECHAR << ");\n";
        else if (std::get<member_type>(member.second) == "unsigned char") ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*BASEUCHAR << ");\n";
        else if (std::get<member_type>(member.second) == "uint8_t")       ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*BASEU8 << ");\n";
        else if (std::get<member_type>(member.second) == "int8_t")        ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASES8 << ");\n";
        else if (std::get<member_type>(member.second) == "uint16_t")      ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*BASEU16 << ");\n";
        else if (std::get<member_type>(member.second) == "int16_t")       ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASES16 << ");\n";
        else if (std::get<member_type>(member.second) == "uint32_t")      ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*BASEU32 << ");\n";
        else if (std::get<member_type>(member.second) == "int32_t")       ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASES32 << ");\n";
        else if (std::get<member_type>(member.second) == "uint64_t")      ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*BASEU64 << ");\n";
        else if (std::get<member_type>(member.second) == "int64_t")       ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASES64 << ");\n";
        else if (std::get<member_type>(member.second) == "float")         ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASEFLOAT << "f);\n";
        else if (std::get<member_type>(member.second) == "double")        ss_ << "in.Set_" << std::get<member_name>(member.second) << "(" << i++*sign*BASEDOUBLE << ");\n";

        sign *= -1;
    }

    return true;
}

}