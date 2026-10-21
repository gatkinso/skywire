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

#pragma once

#include <map>
#include <string>
#include <vector>
#include "generator.h"

namespace skyloupe 
{

class Py3Generator : public Generator
{
public:
    Py3Generator() = delete;
    Py3Generator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~Py3Generator() {}

    bool generate(int& errorcode) override;

protected:
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_includes(int& errorcode);
    virtual bool imp_classname(int& errorcode);
    virtual bool imp_cons(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_equals(int& errorcode);
    virtual bool imp_dunders(int& errorcode);
    virtual bool imp_print(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);

    // struct format for one native member, e.g. ">I" or "16s".
    std::string py_format(const member_desc_t& member);
    // Wire size of a native member in bytes; 0 for user-defined types.
    unsigned int py_size(const member_desc_t& member);

private:
    // One piece of the wire layout, in wire order: a native member, a
    // bitfield storage unit or a nested message.
    struct item_t
    {
        std::string order;      // struct byte order (">", "<", "="); empty if it does not matter
        std::string code;       // struct code without the order, e.g. "H" or "16s"; empty if nested
        unsigned int size = 0;  // bytes; 0 if nested
        std::string value;      // serialize: the value packed
        std::string target;     // deserialize: what the unpacked value is assigned to
        std::string nested;     // the nested message's member name, if nested
        std::string type;       // the nested message's type, if nested
        std::string offset;     // offset from the start of the message, as a Python expression
        std::vector<std::string> after;  // deserialize: assignments after the unpack (bitfields)
    };

    // A run of consecutive items in one byte order, written by its own struct.
    struct run_t
    {
        std::string name;
        std::string order;
        std::string format;
        std::vector<size_t> items;
    };

    std::string classname_;
    std::vector<item_t> items_;
    std::string size_expr_;                 // the message size, as a Python expression
    std::string primary_;                   // the byte order serialize packs in one struct
    std::vector<std::string> orders_;       // byte orders present; deserialize unpacks one struct each
    std::vector<run_t> runs_;               // serialize: the other byte orders' runs
    std::map<std::string, std::string> offset_names_;  // run-time offset expression -> module constant
    std::vector<std::string> offset_order_;            // those expressions, in wire order
    unsigned int run_structs_ = 0;

    bool layout(int& errorcode);
    bool imp_structs(int& errorcode);
    std::string struct_name(const std::string& order) const;
    std::string offset_ref(const std::string& offset) const;
    std::string size_ref() const;
    void tuple_line(const std::string& prefix, const std::vector<std::string>& names, const std::string& suffix);

    void method(const std::string& def_key, const std::string& doc_key = "");
    void line(const std::string& text, unsigned int indents = 2);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string unit_format(const bitfields::unit_t& unit);
    std::string py_fit(const std::string& type, unsigned int bits, const std::string& expr);
    std::string serialize_unit(uint32_t tag);
};

}
