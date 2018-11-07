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

#include <fstream>
#include <sstream>
#include <regex>
#include "cppgenerator.h"

namespace skyloupe
{

CppGenerator::CppGenerator(Stencil& stencil) : Generator(stencil) 
{

}

bool CppGenerator::generate(int& errorcode)
{
    std::cout << "Generating C++" << std::endl;
    errorcode = 0;

    if (false == do_header(errorcode)) return false;
    if (false == do_implementation(errorcode)) return false;
    return true;
}

bool CppGenerator::do_header(int& errorcode)
{
    std::string header_string;

    hdr_banner(header_string, errorcode);
    hdr_sentry(header_string, errorcode);
    hdr_includes(header_string, errorcode);
    hdr_namespace(header_string, errorcode);
    hdr_classname(header_string, errorcode);
    hdr_public(header_string, errorcode);
    hdr_cons(header_string, errorcode);
    hdr_des(header_string, errorcode);
    hdr_functions(header_string, errorcode);
    hdr_members(header_string, errorcode);
    hdr_classend(header_string, errorcode);
    hdr_namespaceend(header_string, errorcode);

    write_file("test.h", header_string, errorcode);

    return true;
}

bool CppGenerator::do_implementation(int& errorcode)
{
    return true;
}

///////////////////////////////////////////////////////////////////////

bool CppGenerator::hdr_banner(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("banner_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_sentry(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("sentry_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_includes(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("include_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_namespace(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("namespace_str");
    std::stringstream ss;

    std::string subst_str = msg_stencil_.get_value_string("namespace");

    value_str = std::regex_replace(value_str, std::regex("__NAMESPACE__"), subst_str);

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_classname(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("classname_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_public(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("public_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_cons(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("cons_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_des(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("des_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_functions(std::string& str, int& errorcode)
{
    std::string value_str = "        //functions";
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_members(std::string& str, int& errorcode)
{
    std::string value_str = "        //memebers";
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_classend(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("endclass_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::hdr_namespaceend(std::string& str, int& errorcode)
{
    std::string value_str = lang_stencil_.get_value_string("endnamespace_str");
    std::stringstream ss;

    ss << value_str << std::endl;

    str.append(ss.str());

    return true;
}

bool CppGenerator::write_file(const std::string& pathname, const std::string& str, int errorcode)
{
    std::ofstream h_file(pathname, std::ofstream::out);

    h_file.write(str.c_str(), str.size());

    return true;
}

}