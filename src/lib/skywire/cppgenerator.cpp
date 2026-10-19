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
#include "cppgenerator.h"
#include "sw_endian.h"

namespace skyloupe
{

bool CppGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();


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

bool CppGenerator::do_header(int& errorcode)
{
    std::string& header_string = results_["header"];

    hdr_banner(errorcode);
    hdr_sentry(errorcode);
    hdr_includes(errorcode);
    hdr_namespace(errorcode);
    hdr_classname(errorcode);
    hdr_public(errorcode);
    hdr_cons(errorcode);
    hdr_des(errorcode);
    hdr_serialize(errorcode);
    hdr_deserialize(errorcode);
    hdr_serialize_pb(errorcode);
    hdr_deserialize_pb(errorcode);
    hdr_size(errorcode);
    hdr_equals(errorcode);
    hdr_get_functions(errorcode);
    hdr_set_functions(errorcode);
    hdr_private(errorcode);
    hdr_members(errorcode);
    hdr_classend(errorcode);
    hdr_namespaceend(errorcode);

    header_string.append(ss_.str());

    return true;
}

bool CppGenerator::do_implementation(int& errorcode)
{
    //For now generating C++ header only code...
    return true;
}

bool CppGenerator::hdr_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

bool CppGenerator::hdr_sentry(int& errorcode)
{
    return gen_one_line("sentry_str");
}

bool CppGenerator::hdr_includes(int& errorcode)
{
    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");

    if (values[0] == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_INCLUDES;
        return false;
    }

    for (auto& item : with_user_headers(values))
    {
        std::string template_str = lang_spec_.get_value_string("include_str");
        std::string target_str = lang_spec_.get_value_string("include_substr");

        template_str = std::regex_replace(template_str, std::regex(target_str), item);

        ss_ << template_str;
    }

    return true;
}

bool CppGenerator::hdr_namespace(int& errorcode)
{
    bool success = gen_one_line_subst("namespace_str", "namespace", "namespace_substr", 0);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_NAMESPACE;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_classname(int& errorcode)
{
    bool success = gen_one_line_subst("classname_str", "classname", "classname_substr", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_C_STRUCTNAME;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_public(int& errorcode)
{
    bool success = gen_one_line("public_str", 1);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_PUBLIC;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_private(int& errorcode)
{
    bool success = gen_one_line("private_str", 1);
    
    if (false == success)
    {
        errorcode |= SW_LANG_CPP_PRIVATE;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_cons(int& errorcode)
{
    bool success = gen_one_line_subst("cons_str", "classname", "classname_substr", 2);

    if (false == success)
    {
        errorcode |= SW_LANG_CPP_CONSTRUCTOR;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_des(int& errorcode)
{
    bool success = gen_one_line_subst("des_str", "classname", "classname_substr", 2);
    
    if (false == success)
    {
        errorcode |= SW_LANG_CPP_DESTRUCTOR;
        return success;
    }

    return success;
}

bool CppGenerator::hdr_get_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_C_HDR_GET;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        insert_indents(2);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("get_string_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("get_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

bool CppGenerator::hdr_set_functions(int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_C_HDR_SET;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        insert_indents(2);
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("set_string_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_bitfield(member.second))
        {
            // Keeps only the field's bits, sign-extending signed ones.
            const std::string& type = std::get<member_type>(member.second);
            template_str = "void Set___MEMBERNAME__(const " + type + " newval) { __MEMBERNAME__ = " +
                           bitfields::c_fit(type, std::get<member_bits>(member.second), "newval") + "; }\n";
        }
        else
        {
            template_str = lang_spec_.get_value_string("set_str");
            template_str = std::regex_replace(template_str, std::regex(type_substr), std::get<member_type>(member.second));
        }

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;
    }

    return true;
}

// serialize() handles PZ mode, then claims the whole SW message from the
// stream at once and writes it with serialize_sw(), which nested messages
// also call to write themselves in place.
bool CppGenerator::hdr_serialize(int& errorcode)
{
    if (false == gen_one_line("serialize_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    gen_lines(lang_spec_.get_value_string("serialize_pz_branch_str"), 3);
    gen_lines(lang_spec_.get_value_string("serialize_sw_call_str"), 3);
    insert_indents(2);
    gen_one_line("endfunction_str");

    gen_lines(lang_spec_.get_value_string("serialize_sw_str"), 2);

    if (false == members_sw("serialize", errorcode))
    {
        return false;
    }

    insert_indents(3);
    ss_ << "return p;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

// deserialize() handles PZ mode, then takes the whole SW message from the
// stream at once (rejecting a short stream before touching any field) and
// reads it with deserialize_sw().
bool CppGenerator::hdr_deserialize(int& errorcode)
{
    if (false == gen_one_line("deserialize_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    std::string pz_branch = lang_spec_.get_value_string("deserialize_pz_branch_str");
    pz_branch = std::regex_replace(pz_branch, std::regex(lang_spec_.get_value_string("classname_substr")), msg_spec_.get_value_string("classname"));
    gen_lines(pz_branch, 3);
    gen_lines(lang_spec_.get_value_string("deserialize_sw_call_str"), 3);
    insert_indents(2);
    gen_one_line("endfunction_str");

    gen_lines(lang_spec_.get_value_string("deserialize_sw_str"), 2);

    if (false == members_sw("deserialize", errorcode))
    {
        return false;
    }

    insert_indents(3);
    ss_ << "return p;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

// The body of serialize_sw() or deserialize_sw(): one line per member, in
// wire order. direction is "serialize" or "deserialize".
bool CppGenerator::members_sw(const std::string& direction, int& errorcode)
{
    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string endian_substr = lang_spec_.get_value_string("endian_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_C_HDR_SERIALIZE;
        return false;
    }

    for (auto& member : ordered_members_)
    {
        if (is_bitfield(member.second))
        {
            if (direction == "serialize")
                write_bitfield(member.first, member.second);
            else
                read_bitfield(member.first, member.second);
            continue;
        }

        std::string template_str;
        if (is_string_type(std::get<member_type>(member.second)))
            template_str = lang_spec_.get_value_string(direction + "_string_mem_str");
        else if (is_native_type(std::get<member_type>(member.second)))
            template_str = lang_spec_.get_value_string(direction + "_native_mem_str");
        else
            template_str = lang_spec_.get_value_string(direction + "_user_mem_str");

        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        template_str = std::regex_replace(template_str, std::regex(endian_substr), endianness(std::get<member_endianness>(member.second)));

        insert_indents(3);
        ss_ << template_str << std::endl;
    }

    return true;
}

std::string CppGenerator::pb_subst(std::string template_str, uint32_t tag, const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const std::string pb_type = lang_spec_.get_value_string("pb_" + type);

    // protobuf wire types: float and double are fixed-width, the integers varints.
    std::string wire_type = "varint";
    if (pb_type == "float")
        wire_type = "fixed32";
    else if (pb_type == "double")
        wire_type = "fixed64";

    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), type);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("length_substr")), std::to_string(std::get<member_length>(member)));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("tag_substr")), std::to_string(tag));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("pbtype_substr")), pb_type);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("wiretype_substr")), wire_type);

    return template_str;
}

// The lang key for a member's protobuf template: direction is "serialize" or "deserialize".
std::string CppGenerator::pb_key(const std::string& direction, const std::string& type)
{
    if (is_string_type(type))
        return direction + "_pb_string_mem_str";
    if (is_user_type(type))
        return direction + "_pb_user_mem_str";
    return direction + "_pb_native_mem_str";
}

// protobuf encoding: each member is the field numbered by its tag, and zero
// values are left out, as proto3 does. A nested message is an embedded
// message, also left out when all its fields are zero.
bool CppGenerator::hdr_serialize_pb(int& errorcode)
{
    gen_one_line("serialize_pb_str", 2);

    for (auto& member : ordered_members_)
    {
        const std::string key = pb_key("serialize", std::get<member_type>(member.second));
        gen_lines(pb_subst(lang_spec_.get_value_string(key), member.first, member.second), 3);
    }

    insert_indents(3);
    ss_ << "return true;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

// protobuf decoding: fields may come in any order or not at all; unknown
// fields, and known ones with an unexpected wire type, are skipped.
bool CppGenerator::hdr_deserialize_pb(int& errorcode)
{
    gen_one_line("deserialize_pb_str", 2);

    gen_lines(lang_spec_.get_value_string("deserialize_pb_loop_begin_str"), 3);

    for (auto& member : ordered_members_)
    {
        std::string template_str = lang_spec_.get_value_string(pb_key("deserialize", std::get<member_type>(member.second)));
        if (is_bitfield(member.second))
        {
            // Out-of-range values from another encoder are cut to the field, as the setter does.
            template_str = lang_spec_.get_value_string("deserialize_pb_bits_mem_str");
            template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("fit_substr")),
                                              bitfields::c_fit(std::get<member_type>(member.second), std::get<member_bits>(member.second), "pbf.get___PBTYPE__()"));
        }
        gen_lines(pb_subst(template_str, member.first, member.second), 3);
    }

    gen_lines(lang_spec_.get_value_string("deserialize_pb_loop_end_str"), 3);
    insert_indents(3);
    ss_ << "return true;" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

bool CppGenerator::hdr_size(int& errorcode)
{
    if (false == gen_one_line("size_str", 2))
    {
        errorcode |= SW_LANG_C_HDR_SIZE;
        return false;
    }

    insert_indents(3);
    ss_ << "return(" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");

    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_SIZE;
        return false;
    }

    std::vector<std::string> terms;
    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_bitfield(member.second))
        {
            // A storage unit counts once, at its first field.
            if (place_of(member.first).first)
                terms.push_back("sizeof(" + bitfields::unsigned_type(unit_of(member.first).type) + ")");
            continue;
        }

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_string_mem_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else if (is_native_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("size_native_mem_str");
            template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_type>(member.second));
        }
        else
        {
            template_str = lang_spec_.get_value_string("size_user_mem_str");
            template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), std::get<member_type>(member.second));
        }

        terms.push_back(template_str);
    }

    for (size_t i = 0; i < terms.size(); i++)
    {
        insert_indents(4);
        ss_ << terms[i] << (i + 1 < terms.size() ? " +" : "") << std::endl;
    }

    insert_indents(3);
    ss_ << ");" << std::endl;
    insert_indents(2);
    gen_one_line("endfunction_str");

    return true;
}

bool CppGenerator::hdr_equals(int& errorcode)
{
    if (false == gen_one_line_subst("equals_str", "classname", "classname_substr", 2))
    {
        errorcode |= SW_LANG_C_HDR_DESERIALIZE;
        return false;
    }

    const std::string equals_selfcheck_str = lang_spec_.get_value_string("equals_selfcheck_str");
    insert_indents(3);
    ss_ << equals_selfcheck_str;
    insert_indents(3);
    ss_ << "return(" << std::endl;

    const std::string member_substr = lang_spec_.get_value_string("member_substr");
    const std::string length_substr = lang_spec_.get_value_string("length_substr");
    if (msg_spec_.get_proto().members_size() == 0 || member_substr == "@NOTFOUND")
    {
        errorcode |= SW_LANG_NOMEMBERS;
        errorcode |= SW_LANG_C_HDR_EQUALS;
        return false;
    }

    int i = 0;
    for (auto& member : ordered_members_)
    {
        std::string template_str;

        if (is_string_type(std::get<member_type>(member.second)))
        {
            template_str = lang_spec_.get_value_string("equals_string_mem_str");
            template_str = std::regex_replace(template_str, std::regex(length_substr), std::to_string(std::get<member_length>(member.second)));
        }
        else
        {
            template_str = lang_spec_.get_value_string("equals_mem_str");
        }

        insert_indents(4);
        template_str = std::regex_replace(template_str, std::regex(member_substr), std::get<member_name>(member.second));
        ss_ << template_str;

        if (i++ < ordered_members_.size() - 1)
        {
            ss_ << " &&";
        }

        ss_ << std::endl;
    }

    insert_indents(3);
    ss_ << ");\n";

    insert_indents(2);
    gen_one_line("endfunction_str");
    return true;
}

bool CppGenerator::hdr_members(int& errorcode)
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
            ss_ << std::get<member_type>(member.second) << " " << std::get<member_name>(member.second) << "{};" << std::endl;
        }
    }

    return true;
}

bool CppGenerator::hdr_classend(int& errorcode)
{
    return gen_one_line("endclass_str", 1);
}

bool CppGenerator::hdr_namespaceend(int& errorcode)
{
    return gen_one_line("endnamespace_str");
}

std::string CppGenerator::endianness(unsigned int endian)
{
    if (endian == SW_BIG_ENDIAN_FLAG)
        return "skyloupe::Endianness::Big";
    if (endian == SW_LITTLE_ENDIAN_FLAG)
        return "skyloupe::Endianness::Little";
    return "skyloupe::Endianness::Native";
}

// A bitfield's storage unit is a local, unit_<n>: its first field declares
// it, each field ors itself in, and the last one writes it.
void CppGenerator::write_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const std::string utype = bitfields::unsigned_type(unit.type);
    const std::string var = "unit_" + std::to_string(place.unit);
    const std::string suffix = bitfields::width(unit.type) == 64 ? "ull" : "u";

    if (place.first)
    {
        insert_indents(3);
        ss_ << utype << " " << var << " = 0;" << std::endl;
    }

    insert_indents(3);
    ss_ << var << " |= static_cast<" << utype << ">((static_cast<" << utype << ">(" << std::get<member_name>(member) << ") & "
        << bitfields::hex(bitfields::mask(place.bits)) << suffix << ") << " << place.shift << ");" << std::endl;

    if (place.last)
    {
        insert_indents(3);
        ss_ << "p = skyloupe::wire::put(p, " << var << ", " << endianness(unit.endian) << ");" << std::endl;
    }
}

void CppGenerator::read_bitfield(uint32_t tag, const member_desc_t& member)
{
    const bitfields::place_t& place = place_of(tag);
    const bitfields::unit_t& unit = unit_of(tag);
    const std::string var = "unit_" + std::to_string(place.unit);

    if (place.first)
    {
        insert_indents(3);
        ss_ << bitfields::unsigned_type(unit.type) << " " << var << " = 0;" << std::endl;
        insert_indents(3);
        ss_ << "p = skyloupe::wire::get(p, " << var << ", " << endianness(unit.endian) << ");" << std::endl;
    }

    insert_indents(3);
    ss_ << std::get<member_name>(member) << " = "
        << bitfields::c_fit(std::get<member_type>(member), place.bits, var + " >> " + std::to_string(place.shift)) << ";" << std::endl;
}

}
