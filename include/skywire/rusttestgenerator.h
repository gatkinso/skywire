//
//   Copyright (C) 2026 Skyloupe, LLC
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

// Generates a test module for a Rust message: #[test] functions run by
// `rustc --test`. The module is a sibling of the message's module.
class RustTestGenerator : public Generator
{
public:
    RustTestGenerator() = delete;
    RustTestGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~RustTestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);
    virtual bool do_sample(int& errorcode);
    // The statement that sets one member of target (an expression such as
    // "s" or "s.inner_mut()") to its shared test value, or nothing for a
    // member with no test value (a nested message).
    std::string set_line(const std::string& target, const member_desc_t& member);
    virtual bool do_expected_wire(int& errorcode);
    virtual bool do_bitfield_setters(int& errorcode);
    virtual bool do_string_setters(int& errorcode);

    // The Rust type of a native member ("u16").
    std::string rust_type(const std::string& type);
    // A byte string literal (b"...") holding value.
    static std::string byte_string(const std::string& value);
};

}
