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
#include "cgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

bool CGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();
    ss_.str("");

    if (false == do_header(errorcode))
    {
        errorcode |= SW_FILE_HEADER;
        return false;
    }

    ss_.str("");

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool CGenerator::do_header(int& errorcode)
{
    std::string& header_string = results_["header"];

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
    hdr_equals_function(errorcode);
    hdr_get_functions(errorcode);
    hdr_set_functions(errorcode);
    hdr_extern_bottom(errorcode);
    hdr_sentry_bottom(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_includes(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals_function(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool CGenerator::hdr_banner(int& errorcode)
{
    return gen_one_line("banner_h_str");
}

bool CGenerator::hdr_sentry_top(int& errorcode)
{
    return gen_one_line_subst("sentry_top_str", "classname", "classname_substr", 0);
}

bool CGenerator::hdr_includes(int& errorcode) {
    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_INCLUDES;
        return false;
    }

    for (auto& item : values)
    {
        std::string template_str = lang_spec_.get_value_string("include_str");
        std::string target_str = lang_spec_.get_value_string("include_substr");

        template_str = std::regex_replace(template_str, std::regex(target_str), item);

        ss_ << template_str;
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
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(2);
        if (is_string_type(std::get<member_type>(member.second)))
        {
            std::string template_str = lang_spec_.get_value_string("member_string_decl_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
            ss_ << template_str << std::endl;
        }
        else
        {
            ss_ << std::get<member_type>(member.second) << " " << std::get<member_name>(member.second) << ";" << std::endl;
        }
    }

    return true;
}

bool CGenerator::hdr_structtype(int& errorcode)
{
    bool success = gen_one_line_subst("classname_bottom_str", "classname", "classname_substr", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    return success;
}

bool CGenerator::hdr_serialize(int& errorcode)
{
    bool success = gen_one_line_subst("serialize_decl_str", "classname", "classname_substr", 1);
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return success;
    }

    return success;
}

bool CGenerator::hdr_deserialize(int& errorcode)
{
    bool success = gen_one_line_subst("deserialize_decl_str", "classname", "classname_substr", 1);
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_DESERIALIZE;
        return success;
    }

    return success;
}

bool CGenerator::hdr_size(int& errorcode)
{
    bool success = gen_one_line_subst("size_str", "classname", "classname_substr", 1);
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_SIZE;
        return success;
    }

    ss_ << lang_spec_.get_value_string("semicolon_str") << std::endl;

    return success;
}

bool CGenerator::hdr_get_functions(int& errorcode)
{
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_GET;
        return false;
    }

    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string key = is_string_type(std::get<member_type>(member.second)) ? "get_string_str" : "get_str";
        std::string template_str = lang_spec_.get_value_string(key);
        template_str = std::regex_replace(template_str, std::regex(classname_substr), msg_spec_.get_value_string("classname"));
        template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool CGenerator::hdr_set_functions(int& errorcode)
{
    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_SET;
        return false;
    }

    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");

    for (auto& member : ordered_members_)
    {
        insert_indents(1);
        std::string key = is_string_type(std::get<member_type>(member.second)) ? "set_string_str" : "set_str";
        std::string template_str = lang_spec_.get_value_string(key);
        template_str = std::regex_replace(template_str, std::regex(classname_substr), msg_spec_.get_value_string("classname"));
        template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool CGenerator::hdr_equals_function(int& errorcode)
{
    bool success = gen_one_line_subst("equals_str", "classname", "classname_substr", 1);
    if (false == success)
    {
        errorcode |= SW_LANG_C_HDR_EQUALS;
        return success;
    }

    ss_ << lang_spec_.get_value_string("semicolon_str") << std::endl;

    return success;
}

bool CGenerator::hdr_extern_bottom(int& errorcode)
{
    return gen_one_line("extern_bottom_str");
}

bool CGenerator::hdr_sentry_bottom(int& errorcode)
{
    return gen_one_line_subst("sentry_bottom_str", "classname", "classname_substr", 1);
}

bool CGenerator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_c_str");
}

bool CGenerator::imp_includes(int& errorcode)
{
    std::string value = msg_spec_.get_value_string("outputheaderfilename", "@NOTFOUND");

    if (value == "@NOTFOUND")
    {
        errorcode = SW_LANG_C_INCLUDES;
        return false;
    }

    bool success = gen_one_line_subst("include_c_str", "outputheaderfilename", "include_substr", 0);

    return success;
}

bool CGenerator::imp_serialize(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string null_check_str = lang_spec_.get_value_string("struct_str_null_check_str");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    bool success = gen_one_line_subst("serialize_impl_str", "classname", "classname_substr", 0);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_SERIALIZE;
        return success;
    }
 
    insert_indents(1);
    ss_ << null_check_str;

    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_spec_.get_value_string("write_native_template_str");
        insert_indents(1);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        if (is_string_type(std::get<member_type>(member.second)))
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_NATIVE_ENDIAN_FLAG");
        else if (std::get<member_endianness>(member.second) == SW_BIG_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_BIG_ENDIAN_FLAG");
        else if (std::get<member_endianness>(member.second) == SW_LITTLE_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_LITTLE_ENDIAN_FLAG");
        else
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_NATIVE_ENDIAN_FLAG");

        ss_ << template_str;
        ss_ << std::endl;
    }

    insert_indents(1);
    ss_ << lang_spec_.get_value_string("endfunction_success_str");

    return success;
}

bool CGenerator::imp_deserialize(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string null_check_str = lang_spec_.get_value_string("struct_str_null_check_str");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    bool success = gen_one_line_subst("deserialize_impl_str", "classname", "classname_substr", 0);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_DESERIALIZE;
        return success;
    }

    insert_indents(1);
    ss_ << null_check_str;

    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_spec_.get_value_string("read_native_template_str");
        insert_indents(1);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        if (is_string_type(std::get<member_type>(member.second)))
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_NATIVE_ENDIAN_FLAG");
        else if (std::get<member_endianness>(member.second) == SW_BIG_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_BIG_ENDIAN_FLAG");
        else if (std::get<member_endianness>(member.second) == SW_LITTLE_ENDIAN_FLAG)
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_LITTLE_ENDIAN_FLAG");
        else
            template_str = std::regex_replace(template_str, std::regex(endian_substr), "SW_NATIVE_ENDIAN_FLAG");

        ss_ << template_str;
        ss_ << std::endl;
    }

    insert_indents(1);
    ss_ << lang_spec_.get_value_string("endfunction_success_str");

    return success;
}

bool CGenerator::imp_size(int& errorcode)
{
    bool success = gen_one_line_subst("size_str", "classname", "classname_substr", 0);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_SIZE;
        return success;
    }

    ss_ << " {" << std::endl;

    insert_indents(1);
    ss_ << "return (" << std::endl;

    int i = 0;
    for (auto& member : ordered_members_)
    {
        insert_indents(2);
        if (is_string_type(std::get<member_type>(member.second)))
            ss_ << std::get<member_length>(member.second);
        else
            ss_ << "sizeof(" << std::get<member_type>(member.second) << ")";

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << "+";
        }

        ss_ << std::endl;
    }

    insert_indents(1);
    ss_ << ");" << std::endl;
   
    gen_one_line("endfunction_str");
    
    return success;
}

bool CGenerator::imp_equals_function(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string null_check_str = lang_spec_.get_value_string("struct_equals_null_check_str");
    bool success = gen_one_line_subst("equals_str", "classname", "classname_substr", 0);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_EQUALS;
        return success;
    }

    ss_ << " {" << std::endl;

    insert_indents(1);
    ss_ << null_check_str;

    insert_indents(1);
    ss_ << "if(" << std::endl;

    const std::string length_substr = lang_spec_.get_value_string("length_substr");
    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str;
        insert_indents(2);
        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("equal_check_string_template_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("equal_check_template_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        }

        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << "&&";
        }

        ss_ << std::endl;
    }

    insert_indents(1);
    ss_ << ")" << std::endl;
    insert_indents(2);
    //using the failure flag for success (and vice versa) in this case.
    ss_ << lang_spec_.get_value_string("return_failure_str");
    insert_indents(1);
    ss_ << lang_spec_.get_value_string("endfunction_success_str");

    return success;
}

bool CGenerator::imp_get_functions(int& errorcode)
{
    bool success = getset_functions("get_c_str", "get_assignment_str", errorcode);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_GET;
        return success;
    }

    return success;
}

bool CGenerator::imp_set_functions(int& errorcode)
{
    bool success = getset_functions("set_c_str", "set_assignment_str", errorcode);
    if (false == success)
    {
        errorcode |= SW_LANG_C_IMP_SET;
        return success;
    }

    return success;
}

bool CGenerator::getset_functions(const std::string& fcn_key, const std::string& ass_key, int& errorcode)
{
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");
    const std::string null_check_str = lang_spec_.get_value_string("struct_value_null_check_str");

    const std::string str_fcn_key = (fcn_key == "get_c_str") ? "get_string_c_str" : "set_string_c_str";
    const std::string str_ass_key = (ass_key == "get_assignment_str") ? "get_string_assignment_str" : "set_string_assignment_str";

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        bool isstr = is_string_type(std::get<member_type>(member.second));

        std::string template_str = lang_spec_.get_value_string(isstr ? str_fcn_key : fcn_key);
        template_str = std::regex_replace(template_str, std::regex(classname_substr), msg_spec_.get_value_string("classname"));
        template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));

        ss_ << template_str;
        ss_ << " {" << std::endl;
        insert_indents(1);
        ss_ << null_check_str;

        std::string ass_str = lang_spec_.get_value_string(isstr ? str_ass_key : ass_key);
        ass_str = std::regex_replace(ass_str, std::regex(member_substr), std::get<member_name>(member.second));
        if (isstr)
            ass_str = std::regex_replace(ass_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));

        insert_indents(1);
        ss_ << ass_str << ";" << std::endl;
        insert_indents(1);
        ss_ << lang_spec_.get_value_string("endfunction_success_str");
    }

    return true;
}

}
