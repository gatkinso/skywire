//
//   Copyright (C) 2026 Geoffrey Atkinson
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

#include <string>
#include "generator.h"

namespace skyloupe
{

// Generates a portable C# 7.3 class: byte[] and offsets with explicit byte
// shifts, so it builds on .NET Framework 4.6.2+, Unity and .NET.
class CSharpGenerator : public Generator
{
public:
    CSharpGenerator(Specification& mspec, Specification& lspec)
        : Generator(mspec, lspec) {}
    virtual ~CSharpGenerator() {}

    bool generate(int& errorcode) override;

    // The bytes a native member type takes on the wire; 0 for strings and
    // user types.
    static unsigned int wire_bytes(const std::string& type);

    // A member name as a C# identifier: keywords get an @ prefix.
    static std::string identifier(const std::string& name);

    // The private field holding a member.
    static std::string field_name(const std::string& name);

protected:
    virtual bool do_implementation(int& errorcode);
    bool check_names(int& errorcode);

    bool uses_inlined_helpers();
    std::string imp_usings();
    std::string imp_class();

    std::string imp_fields();
    std::string imp_properties();
    std::string imp_size();
    std::string imp_serialize();
    std::string imp_deserialize();
    std::string imp_equals();
    std::string imp_hashcode();
    std::string imp_tostring();
    std::string imp_helpers();
    std::string imp_text();
    std::string text_member(const std::string& function, const member_desc_t& member);

    bool wants_text() const { return wants_json() || wants_xml() || wants_bson(); }

    std::string cs_type(const std::string& type);
    std::string cs_endian(unsigned int endian_flag);

private:
    // Where the next fixed-size member goes: o + displacement.
    struct Cursor
    {
        explicit Cursor(CSharpGenerator& generator)
            : gen(generator) {}

        // The code to declare or reload o, if needed; sets index to the
        // member's position and moves past its bytes.
        std::string at(unsigned int bytes);
        // The code to store o + displacement back to offset.
        std::string store();

        CSharpGenerator& gen;
        std::string index;
        unsigned int displacement = 0;
        bool declared = false;
        bool loaded = false;
    };

    std::string classname_;

    std::string value(const std::string& key);
    std::string block(const std::string& block_id, const std::string& body);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string replace(std::string template_str, const std::string& substr_key, const std::string& with);
    std::string to_wire(const member_desc_t& member);
    std::string from_wire(const member_desc_t& member, const std::string& wire);
    std::string read(unsigned int bytes, unsigned int endian, const std::string& at);
    std::string helper(std::string template_str, unsigned int bytes, unsigned int endian);
    std::string serialize_bitfield(uint32_t tag, const member_desc_t& member, Cursor& cursor);
    std::string deserialize_bitfield(uint32_t tag, const member_desc_t& member, Cursor& cursor);
    std::string set_bitfield(const member_desc_t& member);
};

}
