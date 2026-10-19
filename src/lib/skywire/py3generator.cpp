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

#include <algorithm>
#include <regex>
#include <map>
#include <set>
#include <string>
#include "py3generator.h"
#include "sw_endian.h"

namespace skyloupe
{

std::string Py3Generator::py_format(const member_desc_t& member)
{
    const std::string& type = std::get<member_type>(member);

    if (is_string_type(type))
    {
        return std::to_string(std::get<member_length>(member)) + "s";
    }

    // An explicit byte order also selects standard sizes and no alignment.
    std::string prefix;
    switch (std::get<member_endianness>(member))
    {
    case SW_BIG_ENDIAN_FLAG:    prefix = lang_spec_.get_value_string("big_endian_str"); break;
    case SW_LITTLE_ENDIAN_FLAG: prefix = lang_spec_.get_value_string("little_endian_str"); break;
    default:                    prefix = lang_spec_.get_value_string("native_endian_str"); break;
    }

    return prefix + lang_spec_.get_value_string(type);
}

unsigned int Py3Generator::py_size(const member_desc_t& member)
{
    static const std::map<std::string, unsigned int> sizes = {
        { "b", 1 }, { "B", 1 }, { "h", 2 }, { "H", 2 }, { "i", 4 }, { "I", 4 },
        { "q", 8 }, { "Q", 8 }, { "f", 4 }, { "d", 8 },
    };

    const std::string& type = std::get<member_type>(member);

    if (is_string_type(type))
    {
        return std::get<member_length>(member);
    }
    if (!is_native_type(type))
    {
        return 0;
    }

    auto it = sizes.find(lang_spec_.get_value_string(type));
    return it == sizes.end() ? 0 : it->second;
}

std::string Py3Generator::subst(std::string template_str, const member_desc_t& member)
{
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("classname_substr")), classname_);
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("member_substr")), std::get<member_name>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("type_substr")), std::get<member_type>(member));
    template_str = std::regex_replace(template_str, std::regex(lang_spec_.get_value_string("length_substr")), std::to_string(std::get<member_length>(member)));

    return template_str;
}

void Py3Generator::method(const std::string& def_key, const std::string& doc_key)
{
    ss_ << std::endl;
    insert_indents(1);
    ss_ << lang_spec_.get_value_string(def_key);

    if (!doc_key.empty())
    {
        line(lang_spec_.get_value_string(doc_key));
    }
}

void Py3Generator::line(const std::string& text, unsigned int indents)
{
    insert_indents(indents);
    ss_ << text;
}

bool Py3Generator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    if (msg_spec_.get_proto().members_size() == 0)
    {
        errorcode |= SW_LANG_NOMEMBERS;
        return false;
    }

    ss_.str("");

    if (false == layout(errorcode))
    {
        return false;
    }

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool Py3Generator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];

    imp_banner(errorcode);
    imp_includes(errorcode);
    imp_structs(errorcode);
    imp_classname(errorcode);
    imp_cons(errorcode);
    imp_serialize(errorcode);
    imp_deserialize(errorcode);
    imp_size(errorcode);
    imp_equals(errorcode);
    imp_dunders(errorcode);
    imp_print(errorcode);
    imp_get_functions(errorcode);
    imp_set_functions(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool Py3Generator::imp_banner(int& errorcode)
{
    return gen_one_line("banner_str");
}

// struct is always needed; the spec may list more modules.
bool Py3Generator::imp_includes(int& errorcode)
{
    std::set<std::string> modules = { "struct" };

    const std::vector<std::string> values = msg_spec_.get_value("includes", "@NOTFOUND");
    for (auto& item : values)
    {
        if (item != "@NOTFOUND")
        {
            modules.insert(item);
        }
    }

    const std::string include_str = lang_spec_.get_value_string("include_str");
    const std::string include_substr = lang_spec_.get_value_string("include_substr");

    for (auto& module : modules)
    {
        ss_ << std::regex_replace(include_str, std::regex(include_substr), module);
    }

    // Each nested message type comes from its own generated module.
    const std::string user_import_str = lang_spec_.get_value_string("user_import_str");
    const std::string type_substr = lang_spec_.get_value_string("type_substr");

    for (auto& type : user_types())
    {
        ss_ << std::regex_replace(user_import_str, std::regex(type_substr), type);
    }

    return true;
}

bool Py3Generator::imp_classname(int& errorcode)
{
    if (false == gen_one_line_subst("classname_str", "classname", "classname_substr", 0))
    {
        return false;
    }

    ss_ << std::endl;
    line("SIZE = " + size_expr_ + "\n", 1);

    return true;
}

bool Py3Generator::imp_cons(int& errorcode)
{
    method("cons_str");

    const std::string init_substr = lang_spec_.get_value_string("init_substr");

    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string init_key;

        if (is_string_type(type))
            init_key = "init_string_str";
        else if (!is_native_type(type))
            init_key = "init_user_str";
        else if (type == "float" || type == "double")
            init_key = "init_float_str";
        else
            init_key = "init_int_str";

        std::string template_str = lang_spec_.get_value_string("cons_member_init_str");
        template_str = std::regex_replace(template_str, std::regex(init_substr), lang_spec_.get_value_string(init_key));
        line(subst(template_str, member.second));
    }

    return true;
}

// The wire layout. Byte order only matters for 2, 4 and 8 byte values, so
// single bytes, fixed-length strings and 8-bit bitfield units go with
// whichever struct is handy. Deserialize unpacks one struct per byte order,
// each covering the whole message with pad bytes ("x") over everything it
// does not read. Serialize packs the primary byte order the same way, then
// each run of another byte order with its own struct: packing pads writes
// zeros, so only one struct may cover the whole message.
bool Py3Generator::layout(int& errorcode)
{
    items_.clear();
    runs_.clear();
    orders_.clear();
    offset_names_.clear();
    offset_order_.clear();
    run_structs_ = 0;

    unsigned int fixed = 0;
    std::string dynamic;

    for (auto& member : ordered_members_)
    {
        const std::string offset = std::to_string(fixed) + dynamic;
        const std::string& name = std::get<member_name>(member.second);
        const std::string& type = std::get<member_type>(member.second);
        item_t item;
        item.offset = offset;

        if (is_bitfield(member.second))
        {
            const bitfields::place_t& place = place_of(member.first);
            const std::string var = "unit_" + std::to_string(place.unit);
            const std::string field = place.shift ? "(" + var + " >> " + std::to_string(place.shift) + ")" : var;
            const std::string assign = "self." + name + " = " + py_fit(type, place.bits, field);
            if (!place.first)
            {
                // The unit was added at its first field.
                items_.back().after.push_back(assign);
                continue;
            }
            const bitfields::unit_t& unit = unit_of(member.first);
            const std::string format = unit_format(unit);
            item.size = bitfields::width(unit.type) / 8;
            item.order = item.size > 1 ? format.substr(0, 1) : "";
            item.code = format.substr(1);
            item.value = serialize_unit(member.first);
            item.target = var;
            item.after.push_back(assign);
            fixed += item.size;
        }
        else if (is_native_type(type))
        {
            const std::string format = py_format(member.second);
            item.size = py_size(member.second);
            if (is_string_type(type) || item.size == 1)
            {
                item.code = is_string_type(type) ? format : format.substr(1);
            }
            else
            {
                item.order = format.substr(0, 1);
                item.code = format.substr(1);
            }
            item.value = "self." + name;
            item.target = "self." + name;
            fixed += item.size;
        }
        else
        {
            item.nested = name;
            item.type = type;
            dynamic += " + " + type + ".SIZE";
        }

        items_.push_back(item);
    }

    size_expr_ = std::to_string(fixed) + dynamic;

    // The byte orders present, in order of first use.
    for (auto& item : items_)
    {
        if (!item.order.empty() && std::find(orders_.begin(), orders_.end(), item.order) == orders_.end())
        {
            orders_.push_back(item.order);
        }
    }

    // Runs of consecutive items in each byte order.
    std::map<std::string, std::vector<std::vector<size_t>>> runs;
    std::string current;
    for (size_t i = 0; i < items_.size(); i++)
    {
        const std::string& order = items_[i].order;
        if (order.empty())
        {
            current.clear();
            continue;
        }
        if (order != current)
        {
            runs[order].emplace_back();
            current = order;
        }
        runs[order].back().push_back(i);
    }

    // The byte order with the most runs is packed in one struct; the others
    // are left with as few runs as possible. With none, use big endian.
    primary_ = lang_spec_.get_value_string("big_endian_str");
    size_t most = 0;
    for (auto& order : orders_)
    {
        if (runs[order].size() > most)
        {
            most = runs[order].size();
            primary_ = order;
        }
    }
    if (orders_.empty())
    {
        orders_.push_back(primary_);
    }
    else
    {
        // The primary struct also carries every item whose order does not matter.
        orders_.erase(std::find(orders_.begin(), orders_.end(), primary_));
        orders_.insert(orders_.begin(), primary_);
    }

    for (auto& order : orders_)
    {
        if (order == primary_)
        {
            continue;
        }
        for (auto& items : runs[order])
        {
            // Runs with the same format share a struct.
            std::string format = order;
            for (size_t i : items)
            {
                format += items_[i].code;
            }
            std::string name;
            for (auto& run : runs_)
            {
                if (run.format == format)
                {
                    name = run.name;
                }
            }
            if (name.empty())
            {
                name = struct_name(order) + "_" + std::to_string(run_structs_++);
            }
            runs_.push_back({ name, order, format, items });
        }
    }

    // Offsets that include a nested message's size become module constants,
    // computed once at import.
    for (size_t i = 0; i < items_.size(); i++)
    {
        const std::string& offset = items_[i].offset;
        bool used = !items_[i].nested.empty();
        for (auto& run : runs_)
        {
            used = used || run.items.front() == i;
        }
        if (used && offset.find('.') != std::string::npos && offset_names_.count(offset) == 0)
        {
            const std::string name = "_OFFSET_" + std::to_string(offset_names_.size());
            offset_names_[offset] = name;
            offset_order_.push_back(offset);
        }
    }

    return true;
}

// Whole-message struct for one byte order: _BE, _LE or _NE.
std::string Py3Generator::struct_name(const std::string& order) const
{
    if (order == ">")
        return "_BE";
    if (order == "<")
        return "_LE";
    return "_NE";
}

// An offset as written in a method: a literal, or a module constant for one
// that includes a nested message's size.
std::string Py3Generator::offset_ref(const std::string& offset) const
{
    auto it = offset_names_.find(offset);
    return it == offset_names_.end() ? offset : it->second;
}

// The message size as written in a method: a literal, or the class constant.
std::string Py3Generator::size_ref() const
{
    return size_expr_.find('.') == std::string::npos ? size_expr_ : "self.SIZE";
}

// prefix(names...)suffix, one name per line when there is more than one.
void Py3Generator::tuple_line(const std::string& prefix, const std::vector<std::string>& names, const std::string& suffix)
{
    if (names.size() == 1)
    {
        line(prefix + names[0] + suffix + "\n");
        return;
    }
    line(prefix.substr(0, prefix.find_last_not_of(' ') + 1) + "\n");
    for (auto& name : names)
    {
        line(name + ",\n", 3);
    }
    line(suffix.substr(suffix.find_first_not_of(',')) + "\n");
}

// The module-level structs and offsets the methods use.
bool Py3Generator::imp_structs(int& errorcode)
{
    ss_ << std::endl;

    for (auto& order : orders_)
    {
        std::string format = order;
        unsigned int pad = 0;
        bool dynamic = false;
        for (auto& item : items_)
        {
            const bool mine = item.order == order || (item.order.empty() && !item.code.empty() && order == primary_);
            if (mine)
            {
                if (pad)
                {
                    format += std::to_string(pad) + "x";
                    pad = 0;
                }
                format += item.code;
            }
            else if (!item.nested.empty())
            {
                if (pad)
                {
                    format += std::to_string(pad) + "x";
                    pad = 0;
                }
                format += "{" + item.type + ".SIZE}x";
                dynamic = true;
            }
            else
            {
                pad += item.size;
            }
        }
        // Trailing pad bytes keep every struct the size of the whole message.
        if (pad)
        {
            format += std::to_string(pad) + "x";
        }
        ss_ << struct_name(order) << " = struct.Struct(" << (dynamic ? "f" : "") << "\"" << format << "\")" << std::endl;
    }

    std::set<std::string> written;
    for (auto& run : runs_)
    {
        if (written.insert(run.name).second)
        {
            ss_ << run.name << " = struct.Struct(\"" << run.format << "\")" << std::endl;
        }
    }

    for (auto& offset : offset_order_)
    {
        ss_ << offset_names_.at(offset) << " = " << offset << std::endl;
    }

    return true;
}

bool Py3Generator::imp_serialize(int& errorcode)
{
    method("serialize_fcn_str", "serialize_doc_str");

    // One struct covers a message with no nested message and one byte order.
    bool nested = false;
    for (auto& item : items_)
    {
        nested = nested || !item.nested.empty();
    }
    std::vector<std::string> values;
    for (auto& item : items_)
    {
        if (item.order == primary_ || (item.order.empty() && !item.code.empty()))
        {
            values.push_back(item.value);
        }
    }

    if (!nested && runs_.empty())
    {
        tuple_line("return " + struct_name(primary_) + ".pack(", values, ")");
    }
    else
    {
        line("buffer = bytearray(" + size_ref() + ")\n");
        line("self._serialize_into(buffer, 0)\n");
        line("return bytes(buffer)\n");
    }

    ss_ << std::endl;
    line("def _serialize_into(self, buffer, offset):\n", 1);
    line("\"\"\"Write the wire representation into buffer at offset.\"\"\"\n");
    tuple_line(struct_name(primary_) + ".pack_into(buffer, offset, ", values, ")");

    for (size_t i = 0; i < items_.size(); i++)
    {
        const item_t& item = items_[i];
        if (!item.nested.empty())
        {
            line("self." + item.nested + "._serialize_into(buffer, offset + " + offset_ref(item.offset) + ")\n");
            continue;
        }
        for (auto& run : runs_)
        {
            if (run.items.front() != i)
            {
                continue;
            }
            std::vector<std::string> run_values;
            for (size_t j : run.items)
            {
                run_values.push_back(items_[j].value);
            }
            tuple_line(run.name + ".pack_into(buffer, offset + " + offset_ref(item.offset) + ", ", run_values, ")");
        }
    }

    return true;
}

bool Py3Generator::imp_deserialize(int& errorcode)
{
    method("deserialize_fcn_str", "deserialize_doc_str");
    const std::string size = size_ref();
    line("if len(buffer) < " + size + ":\n");
    line("raise ValueError(\"" + classname_ + ": need %d bytes, have %d\" % (" + size + ", len(buffer)))\n", 3);
    line("self._deserialize_from(buffer, 0)\n");

    ss_ << std::endl;
    line("def _deserialize_from(self, buffer, offset):\n", 1);
    line("\"\"\"Read the wire representation from buffer at offset.\"\"\"\n");

    for (auto& order : orders_)
    {
        std::vector<std::string> targets;
        for (auto& item : items_)
        {
            if (item.order == order || (item.order.empty() && !item.code.empty() && order == primary_))
            {
                targets.push_back(item.target);
            }
        }
        if (targets.empty())
        {
            continue;
        }
        tuple_line("(", targets, ",) = " + struct_name(order) + ".unpack_from(buffer, offset)");
    }

    for (auto& item : items_)
    {
        if (!item.nested.empty())
        {
            line("self." + item.nested + "._deserialize_from(buffer, offset + " + offset_ref(item.offset) + ")\n");
        }
        for (auto& assign : item.after)
        {
            line(assign + "\n");
        }
    }

    return true;
}

bool Py3Generator::imp_size(int& errorcode)
{
    method("size_fcn_str", "size_doc_str");
    line("return self.SIZE\n");

    return true;
}

bool Py3Generator::imp_equals(int& errorcode)
{
    method("equals_fcn_str");
    line(lang_spec_.get_value_string("equals_start"));

    bool first = true;
    for (auto& member : ordered_members_)
    {
        std::string key = first ? "equals_first_mem_str" : "equals_mem_str";
        line(subst(lang_spec_.get_value_string(key), member.second), 3);
        first = false;
    }

    line(lang_spec_.get_value_string("equals_end"));

    return true;
}

bool Py3Generator::imp_dunders(int& errorcode)
{
    const std::string classname_substr = lang_spec_.get_value_string("classname_substr");

    method("eq_fcn_str");
    line(std::regex_replace(lang_spec_.get_value_string("eq_body_str"), std::regex(classname_substr), classname_));

    method("repr_fcn_str");
    line(std::regex_replace(lang_spec_.get_value_string("repr_body_str"), std::regex(classname_substr), classname_));

    return true;
}

bool Py3Generator::imp_print(int& errorcode)
{
    method("print_fcn_str");

    for (auto& member : ordered_members_)
    {
        line(subst(lang_spec_.get_value_string("print_mem_str"), member.second));
    }

    return true;
}

bool Py3Generator::imp_get_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        ss_ << std::endl;
        line(subst(lang_spec_.get_value_string("get_fcn_str"), member.second), 1);
        line(subst(lang_spec_.get_value_string("get_body_str"), member.second));
    }

    return true;
}

bool Py3Generator::imp_set_functions(int& errorcode)
{
    for (auto& member : ordered_members_)
    {
        const std::string& type = std::get<member_type>(member.second);
        std::string body_key;

        if (is_string_type(type))
            body_key = "set_string_body_str";
        else if (type == "float")
            body_key = "set_float_body_str";   // round to the float32 that goes on the wire
        else
            body_key = "set_body_str";

        ss_ << std::endl;
        line(subst(lang_spec_.get_value_string("set_fcn_str"), member.second), 1);
        if (is_bitfield(member.second))
        {
            // Keeps only the field's bits, sign-extending signed ones.
            line("self." + std::get<member_name>(member.second) + " = " + py_fit(type, std::get<member_bits>(member.second), "newval") + "\n");
            continue;
        }
        line(subst(lang_spec_.get_value_string(body_key), member.second));
    }

    return true;
}

// The struct format of a bitfield storage unit: its byte order and the
// unsigned type of its width (fields are packed as bits).
std::string Py3Generator::unit_format(const bitfields::unit_t& unit)
{
    member_desc_t desc{ bitfields::unsigned_type(unit.type), "", unit.endian, 0, 0 };
    return py_format(desc);
}

// expr's low bits bits as the field's value: masked, and sign-extended for
// signed types with (v ^ sign) - sign.
std::string Py3Generator::py_fit(const std::string& type, unsigned int bits, const std::string& expr)
{
    const std::string masked = "(" + expr + " & " + bitfields::hex(bitfields::mask(bits)) + ")";
    if (false == bitfields::is_signed(type))
    {
        return masked;
    }
    const std::string sign = bitfields::hex(uint64_t(1) << (bits - 1));
    return "(" + masked + " ^ " + sign + ") - " + sign;
}

// One storage unit's value: every field's bits, each at its shift.
std::string Py3Generator::serialize_unit(uint32_t tag)
{
    const bitfields::unit_t& unit = unit_of(tag);

    std::string value;
    for (uint32_t field_tag : unit.tags)
    {
        const bitfields::place_t& place = place_of(field_tag);
        std::string field = "(self." + std::get<member_name>(ordered_members_.at(field_tag)) + " & " + bitfields::hex(bitfields::mask(place.bits)) + ")";
        if (place.shift)
        {
            field = "(" + field + " << " + std::to_string(place.shift) + ")";
        }
        value += (value.empty() ? "" : " | ") + field;
    }

    return value;
}

}
