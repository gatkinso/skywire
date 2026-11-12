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
#include <functional>
#include <vector>
#include "generator.h"

namespace skyloupe
{

// Generates one Rust module per message: a struct with private fields,
// accessors, and serialize/deserialize to the fixed-size wire format. The
// module is a file named after the message in snake_case; nested message
// types are its siblings (use super::<module>::<Type>).
class RustGenerator : public Generator
{
public:
    RustGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~RustGenerator() {}

    bool generate(int& errorcode) override;

    // A member name as a Rust identifier: a raw identifier (r#type) if it is
    // a keyword.
    static std::string identifier(const std::string& name);

    // The module (file name without .rs) holding a generated message type.
    static std::string module_name(const std::string& type);

    // Rust's own naming lints, which the generated code must not trip.
    static bool is_snake_case(const std::string& name);
    static bool is_camel_case(const std::string& name);

protected:
    virtual bool do_implementation(int& errorcode);

    // Rejects, naming them on stderr, names Rust can't take: self, Self,
    // super, crate and _; and members whose accessors would collide with
    // the generated methods or with each other.
    bool check_names(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_lints(int& errorcode);
    virtual bool imp_uses(int& errorcode);
    virtual bool imp_struct(int& errorcode);
    virtual bool imp_default(int& errorcode);
    virtual bool imp_impl(int& errorcode);

    std::string rust_type(const std::string& type);
    std::string rust_endian(unsigned int endian_flag);
    unsigned int rust_bits(const std::string& type);
    bool is_float_type(const std::string& type);
    // The function names a member's accessors take.
    std::vector<std::string> accessor_names(const member_desc_t& member);

    bool wants_text() const { return wants_json() || wants_xml() || wants_bson(); }
    // The items --generate_json, --generate_xml and --generate_bson add to
    // the impl block, and one member's lines in them.
    std::vector<std::string> text_items();
    std::string text_member(const std::string& function, const member_desc_t& member);

private:
    std::string classname_;

    bool do_block(const std::string block_id, std::function<std::string()> builder);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string size_expression();
    std::string serialize_body();
    std::string deserialize_body();
    std::string accessors(const member_desc_t& member);
    std::string serialize_bitfield(uint32_t tag, const member_desc_t& member);
    std::string deserialize_bitfield(uint32_t tag, const member_desc_t& member);
};

}
