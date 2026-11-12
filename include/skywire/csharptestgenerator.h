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

class CSharpTestGenerator : public Generator
{
public:
    CSharpTestGenerator() = delete;
    CSharpTestGenerator(Specification& mspec, Specification& lspec)
        : Generator(mspec, lspec) {}
    virtual ~CSharpTestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);
    std::string set_line(const std::string& target, const member_desc_t& member);
    std::string expected_wire_test(const std::vector<uint8_t>& wire);
    std::string text_tests_code(const std::string& classname, std::vector<std::string>& tests);

    // The shared test literal for a native type, suffixed or cast to suit the
    // member's C# type (e.g. "3.5f", "unchecked((sbyte)0x25UL)").
    std::string csharp_literal(const std::string& type, const std::string& literal);
};

}
