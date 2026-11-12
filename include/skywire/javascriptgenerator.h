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
#include <vector>
#include "generator.h"

namespace skyloupe
{

// Generates a portable ES2020 module (Uint8Array, DataView, BigInt and
// TextEncoder only, no Node APIs) and its TypeScript declarations: the
// module goes in the implementation slot and the .d.ts in the header slot.
class JavaScriptGenerator : public Generator
{
public:
    JavaScriptGenerator(Specification& mspec, Specification& lspec)
        : Generator(mspec, lspec) {}
    virtual ~JavaScriptGenerator() {}

    bool generate(int& errorcode) override;

    // The DataView accessor suffix of a native member type ("Uint16",
    // "BigInt64", "Float32", ...), or empty for strings and user types.
    static std::string view_type(const std::string& type);

    // The bytes a native member type takes on the wire; 0 for strings and
    // user types.
    static unsigned int wire_bytes(const std::string& type);

    // Whether a native member type is a BigInt in JavaScript (the 64-bit
    // integers).
    static bool is_bigint(const std::string& type);

    // The backing property holding a member.
    static std::string field_name(const std::string& name);

    // The getter that decodes a string member as text.
    static std::string text_name(const std::string& name);

    // The file a nested message type's module is imported from, relative to
    // this one, by the lang file's implementation file pattern.
    std::string module_path(const std::string& type);

protected:
    virtual bool do_implementation(int& errorcode);
    virtual bool do_declarations(int& errorcode);
    bool check_names(int& errorcode);

    std::string imp_imports();
    std::string imp_constants();
    std::string imp_class();

    std::string imp_constructor();
    std::string imp_accessors();
    std::string imp_size();
    std::string imp_serialize();
    std::string imp_deserialize();
    std::string imp_equals();
    std::string imp_tojson();
    std::string imp_tostring();
    std::string imp_helpers();

    std::string dts_accessors();
    std::string dts_json();

private:
    std::string classname_;

    std::string value(const std::string& key);
    std::string block(const std::string& block_id, const std::string& body);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string endian_arg(unsigned int endian_flag);
    std::string setter_wrap(const member_desc_t& member);
    std::string serialize_bitfield(uint32_t tag, const member_desc_t& member, unsigned int& displacement);
    std::string deserialize_bitfield(uint32_t tag, const member_desc_t& member, unsigned int& displacement);
    std::string at(unsigned int displacement);

    bool uses_native_order();
    bool has_strings();

    bool wants_text() const { return wants_json() || wants_xml() || wants_bson(); }
    // The formats asked for: "json", "xml", "bson".
    std::vector<std::string> text_formats() const;
    // The methods --generate_json, --generate_xml and --generate_bson add to
    // the class and its declarations, and one member's lines in them.
    std::string imp_text_functions();
    std::string dts_text_methods();
    std::string text_member(const std::string& function, const member_desc_t& member);
};

}
