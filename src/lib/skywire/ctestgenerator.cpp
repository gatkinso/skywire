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
#include "ctestgenerator.h"

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

             if (std::get<member_type>(member.second) == "char")          ss_ << "out[i]." << std::get<member_name>(member.second) << " = " << (int)i++*BASECHAR << ";\n";
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
    }

    insert_indents(2);
    ss_ << "sign *= -1;\n";

    return true;
}

}
