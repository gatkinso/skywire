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

    hdr_banner(errorcode);
    hdr_sentry_top(errorcode);
    hdr_includes(errorcode);
    hdr_extern_top(errorcode);
    hdr_structtag(errorcode);
    hdr_members(errorcode);
    hdr_structtype(errorcode);
    hdr_serialize(errorcode);
    hdr_deserialize(errorcode);
    hdr_size(errorcode);
    hdr_get_functions(errorcode);
    hdr_set_functions(errorcode);
    hdr_equals_function(errorcode);
    hdr_extern_bottom(errorcode);
    hdr_sentry_bottom(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CGenerator::do_implementation(int& errorcode)
{
    results_.push_back("");

    std::string& imp_string = results_[1];

    imp_banner(errorcode);
    cr();
    imp_includes(errorcode);
    cr();
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);
    imp_equals_function(errorcode);
    cr();

    imp_string.append(ss_.str());

    return true;
}

bool CGenerator::hdr_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool CGenerator::hdr_sentry_top(int& errorcode)
{
    return gen_one_line_subst("sentry_top_str", "classname", "classname_substr", 0);
}

bool CGenerator::hdr_includes(int& errorcode) {
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

bool CGenerator::hdr_extern_top(int& errorcode)
{
    return gen_one_line("extern_top_str");
}

bool CGenerator::hdr_structtag(int& errorcode)
{
    return gen_one_line_subst("classname_top_str", "classname", "classname_substr", 1);
}

bool CGenerator::hdr_members(int& errorcode)
{
    return inorder_members(errorcode, 2);
}

bool CGenerator::hdr_structtype(int& errorcode)
{
    bool success = gen_one_line_subst("classname_bottom_str", "classname", "classname_substr", 1);
    return success;
}

bool CGenerator::hdr_serialize(int& errorcode)
{
    bool success = gen_one_line_subst("serialize_str", "classname", "classname_substr", 1);
    insert_char(';');
    cr();
    return success;
}

bool CGenerator::hdr_deserialize(int& errorcode)
{
    bool success = gen_one_line_subst("deserialize_str", "classname", "classname_substr", 1);
    insert_char(';');
    cr();
    return success;
}

bool CGenerator::hdr_size(int& errorcode)
{
    bool success = gen_one_line_subst("size_str", "classname", "classname_substr", 1);
    insert_char(';');
    cr();
    return success;
}

bool CGenerator::hdr_get_functions(int& errorcode)
{
    return inorder_functions("get_str", errorcode, 1, true);
}
bool CGenerator::hdr_set_functions(int& errorcode)
{
    return inorder_functions("set_str", errorcode, 1, true);
}

bool CGenerator::hdr_equals_function(int& errorcode) {
    bool success = gen_one_line_subst("equals_str", "classname", "classname_substr", 1);
    insert_char(';');
    cr();
    return success;
}

bool CGenerator::hdr_extern_bottom(int& errorcode)
{
    return gen_one_line("extern_bottom_str");
}

bool CGenerator::hdr_sentry_bottom(int& errorcode)
{
    return gen_one_line("sentry_bottom_str");
}

bool CGenerator::imp_banner(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_includes(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_serialize(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_deserialize(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_size(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_get_functions(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_set_functions(int& errorcode) {
    bool success = true;

    return success;
}
bool CGenerator::imp_equals_function(int& errorcode) {
    bool success = true;

    return success;
}

}