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

#include <cstdint>
#include <string>
#include <vector>
#include "generator.h"

namespace skyloupe
{

class JavaScriptTestGenerator : public Generator
{
public:
    JavaScriptTestGenerator() = delete;
    JavaScriptTestGenerator(Specification& mspec, Specification& lspec)
        : Generator(mspec, lspec) {}
    virtual ~JavaScriptTestGenerator() = default;

    bool generate(int& errorcode) override;

    // The shared test literal for a native type as JavaScript: 64-bit
    // integers get BigInt's n suffix (e.g. "-9000000000n", "0x25n").
    static std::string javascript_literal(const std::string& type, const std::string& literal);

protected:
    virtual bool do_test_implementation(int& errorcode);
    std::string set_line(const std::string& target, const member_desc_t& member);
    std::string expected_wire_test(const std::string& classname, const std::vector<uint8_t>& wire);
    // With --generate_json, --generate_xml or --generate_bson: the exact
    // text or bytes, round trips and rejected input.
    std::string text_tests(const std::string& classname);
    std::string bson_tests(const std::string& classname);

    // The module the class is imported from, by the implementation file name.
    std::string module_name();
};

}
