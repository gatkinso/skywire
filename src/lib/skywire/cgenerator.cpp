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

#include "cgenerator.h"

namespace skyloupe
{

bool CGenerator::generate(int& errorcode)
{
    std::cout << "Generating C" << std::endl;
    errorcode = 0;

    results_.clear();

    ss_.str("");
    if (false == do_header(errorcode)) return false;

    ss_.str("");
    if (false == do_implementation(errorcode)) return false;

    return true;
}

bool CGenerator::do_header(int& errorcode)
{
    results_.push_back("");

    std::string& header_string = results_[0];

    hdr_banner(header_string, errorcode);
    cr(header_string);
    hdr_sentry_top(header_string, errorcode);
    cr(header_string);
    hdr_includes(header_string, errorcode);
    cr(header_string);
    hdr_extern_top(header_string, errorcode);
    cr(header_string);
    hdr_structtag(header_string, errorcode);
    hdr_members(header_string, errorcode);
    hdr_structtype(header_string, errorcode);
    cr(header_string);
    hdr_serialize(header_string, errorcode);
    hdr_deserialize(header_string, errorcode);
    hdr_size(header_string, errorcode);
    cr(header_string);
    hdr_get_functions(header_string, errorcode);
    cr(header_string);
    hdr_set_functions(header_string, errorcode);
    cr(header_string);
    hdr_equals_function(header_string, errorcode);
    cr(header_string);
    hdr_extern_bottom(header_string, errorcode);
    cr(header_string);
    hdr_sentry_bottom(header_string, errorcode);
    cr(header_string);

    header_string.append(ss_.str());

    return true;
}

bool CGenerator::do_implementation(int& errorcode)
{
    results_.push_back("");

    std::string& imp_string = results_[1];

    imp_banner(imp_string, errorcode);
    cr(imp_string);
    imp_includes(imp_string, errorcode);
    cr(imp_string);
    imp_serialize(imp_string, errorcode);
    imp_deserialize(imp_string, errorcode);
    imp_size(imp_string, errorcode);
    imp_get_functions(imp_string, errorcode);
    imp_set_functions(imp_string, errorcode);
    imp_equals_function(imp_string, errorcode);
    cr(imp_string);

    imp_string.append(ss_.str());

    return true;
}

bool CGenerator::hdr_banner(std::string& str, int& errorcode)
{
    return gen_one_line(str, "banner_str");
}

bool CGenerator::hdr_sentry_top(std::string& str, int& errorcode)
{
    return gen_one_line(str, "sentry_top_str");
}

bool CGenerator::hdr_includes(std::string& str, int& errorcode) {
    std::vector<std::string> values = msg_stencil_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
    {
        errorcode = SW_LANGCPPINCLUDES;
        return false;
    }

    for (auto& item : values)
    {
        ss_ << "#include " << item << std::endl;
    }

    return true;
}

bool CGenerator::hdr_extern_top(std::string& str, int& errorcode)
{
    return gen_one_line(str, "extern_bottom_str");
}

bool CGenerator::hdr_structtag(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_members(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_structtype(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_serialize(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_deserialize(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_size(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_get_functions(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_set_functions(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::hdr_equals_function(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}

bool CGenerator::hdr_extern_bottom(std::string& str, int& errorcode)
{
    return gen_one_line(str, "extern_bottom_str");
}

bool CGenerator::hdr_sentry_bottom(std::string& str, int& errorcode)
{
    return gen_one_line(str, "sentry_bottom_str");
}

bool CGenerator::imp_banner(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_includes(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_serialize(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_deserialize(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_size(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_get_functions(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_set_functions(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_equals_function(std::string& str, int& errorcode) {
    bool success = true;

    return success;
}

}