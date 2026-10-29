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

#include <iostream>
#include <algorithm>
#include "generator.h"
#include "test_values.h"

namespace skyloupe
{

Generator::Generator(Specification& msg_specification, Specification& lang_specification) 
: msg_spec_(msg_specification)
, lang_spec_(lang_specification)
, results_()
{
    ordered_members_ = members_of(msg_spec_);
    bit_layout_ = layout_of(msg_spec_, ordered_members_);

    for (auto& member : lang_spec_.get_proto().native_types()) {
        native_types_.insert(member);
    }
}

Generator::members_t Generator::members_of(Specification& spec)
{
    members_t members;

    for (auto& member : spec.get_proto().members())
    {
        members[member.first] = std::make_tuple(member.second.type(), member.second.name(), member.second.endian(), member.second.length(), member.second.bits());
    }

    return members;
}

bitfields::layout_t Generator::layout_of(Specification& spec, const members_t& members)
{
    std::vector<bitfields::field_t> fields;

    for (auto& member : members)
    {
        fields.push_back({ member.first, std::get<member_type>(member.second), std::get<member_endianness>(member.second),
                           std::get<member_bits>(member.second) });
    }

    return bitfields::layout(fields, spec.get_value_string("bitorder", "lsb") == "msb");
}

void Generator::set_nested_specs(const std::map<std::string, Specification>& specs)
{
    nested_members_.clear();

    for (auto& spec : specs)
    {
        Specification copy = spec.second;
        nested_members_[spec.first] = members_of(copy);
    }
}

bool Generator::for_each_leaf(const leaf_visitor_t& visit)
{
    std::vector<std::string> path;
    return for_each_leaf(ordered_members_, path, visit);
}

bool Generator::for_each_leaf(const members_t& members, std::vector<std::string>& path, const leaf_visitor_t& visit)
{
    for (auto& member : members)
    {
        const std::string& type = std::get<member_type>(member.second);
        path.push_back(std::get<member_name>(member.second));

        if (is_user_type(type))
        {
            auto nested = nested_members_.find(type);
            if (nested == nested_members_.end())
            {
                std::cerr << "nested type '" << type << "' has no specification" << std::endl;
                return false;
            }
            if (false == for_each_leaf(nested->second, path, visit))
            {
                return false;
            }
        }
        else
        {
            visit(path, member.second);
        }

        path.pop_back();
    }

    return true;
}

std::vector<std::string> Generator::nested_types_in_order()
{
    std::vector<std::string> order;

    std::function<void(const members_t&)> visit = [&](const members_t& members) {
        for (auto& member : members)
        {
            const std::string& type = std::get<member_type>(member.second);
            if (is_user_type(type) && std::find(order.begin(), order.end(), type) == order.end())
            {
                auto nested = nested_members_.find(type);
                if (nested != nested_members_.end())
                {
                    visit(nested->second);
                }
                order.push_back(type);
            }
        }
    };
    visit(ordered_members_);

    return order;
}

std::string Generator::leaf_t::join(const std::string& between, const std::string& before, const std::string& after) const
{
    std::string joined;

    for (size_t i = 0; i < path.size(); i++)
    {
        joined += (i == 0 ? "" : between) + before + path[i] + after;
    }

    return joined;
}

std::string Generator::leaf_t::join_parents(const std::string& before, const std::string& after) const
{
    std::string joined;

    for (size_t i = 0; i + 1 < path.size(); i++)
    {
        joined += before + path[i] + after;
    }

    return joined;
}

bool Generator::relay_leaves(std::vector<leaf_t>& leaves, int& errorcode)
{
    bool valued = true;

    bool found = for_each_leaf([&](const std::vector<std::string>& path, const member_desc_t& member) {
        const std::string& type = std::get<member_type>(member);
        if (false == is_string_type(type) && nullptr == test_values::literal(type))
        {
            std::cerr << "member '" << path.back() << "' (" << type << ") has no shared test value" << std::endl;
            valued = false;
        }
        leaves.push_back({ path, member });
    });

    if (false == found || false == valued)
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

std::string Generator::test_literal(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    if (is_bitfield(member))
    {
        return test_values::literal_bits(type, std::get<member_bits>(member));
    }
    const char* literal = test_values::literal(type);
    return literal ? literal : "";
}

std::string Generator::out_of_range_hex(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);
    const unsigned int bits = std::get<member_bits>(member);
    if (bits == 0 || bits >= bitfields::width(type))
    {
        return "";
    }
    return bitfields::hex(test_values::bits_value(type, bits) & bitfields::mask(bits) | (uint64_t(1) << bits));
}

bool Generator::expected_wire(std::vector<uint8_t>& out)
{
    for (auto& member : ordered_members_)
    {
        if (false == is_bitfield(member.second))
        {
            if (false == test_values::encode(std::get<member_type>(member.second), std::get<member_endianness>(member.second),
                                             std::get<member_length>(member.second), out))
            {
                return false;
            }
            continue;
        }

        // A unit is encoded once, at its first field.
        if (false == place_of(member.first).first)
        {
            continue;
        }

        const bitfields::unit_t& unit = unit_of(member.first);
        std::vector<test_values::bitfield_t> fields;
        for (uint32_t tag : unit.tags)
        {
            fields.push_back({ place_of(tag).shift, place_of(tag).bits });
        }
        test_values::encode_unit(unit.type, unit.endian, fields, out);
    }

    return true;
}

bool Generator::is_native_type(const std::string& type)
{
    if (native_types_.end() == native_types_.find(type))
    {
        return false;
    }

    return true;
}

bool Generator::is_string_type(const std::string& type)
{
    return type == "string";
}

bool Generator::is_user_type(const std::string& type)
{
    return false == is_string_type(type) && false == is_native_type(type);
}

std::set<std::string> Generator::user_types()
{
    std::set<std::string> types;

    for (auto& member : ordered_members_)
    {
        if (is_user_type(std::get<member_type>(member.second)))
        {
            types.insert(std::get<member_type>(member.second));
        }
    }

    return types;
}

std::vector<std::string> Generator::with_user_headers(std::vector<std::string> includes)
{
    for (auto& type : user_types())
    {
        const std::string header = "\"" + type + ".h\"";
        if (std::find(includes.begin(), includes.end(), header) == includes.end())
        {
            includes.push_back(header);
        }
    }

    return includes;
}

bool Generator::insert_indents(unsigned int indents)
{
    std::string indent_str = lang_spec_.get_value_string("indent_str");

    for (unsigned int i = 0; i < indents; i++)
    {
        ss_ << indent_str;
    }

    return true;
}

bool Generator::gen_one_line(const std::string& key, unsigned int indents)
{
    std::string value_str = lang_spec_.get_value_string(key);

    insert_indents(indents);

    ss_ << value_str;

    return true;
}

void Generator::gen_lines(std::string text, unsigned int indents)
{
    size_t start = 0;
    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
        {
            end = text.size();
        }
        insert_indents(indents);
        ss_ << text.substr(start, end - start) << std::endl;
        start = end + 1;
    }
}

std::string Generator::replace_all(std::string text, const std::string& token, const std::string& with)
{
    if (token.empty())
    {
        return text;
    }
    for (auto pos = text.find(token); pos != std::string::npos; pos = text.find(token, pos + with.size()))
    {
        text.replace(pos, token.size(), with);
    }
    return text;
}

bool Generator::gen_one_line_subst(const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents)
{
    std::string template_str = lang_spec_.get_value_string(template_key);
    std::string subst_str = msg_spec_.get_value_string(subst_key);
    std::string target_str = lang_spec_.get_value_string(target_key);

    insert_indents(indents);

    template_str = replace_all(template_str, target_str, subst_str);

    ss_ << template_str;

    return true;
}

}