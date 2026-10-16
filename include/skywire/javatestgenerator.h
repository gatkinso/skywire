//
//   Copyright (C) 2019 Skyloupe, LLC
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

class JavaTestGenerator : public Generator
{
public:
    JavaTestGenerator() = delete;
    JavaTestGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~JavaTestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);
    virtual bool do_sets(int& errorcode);
    std::string set_line(const std::string& target, const member_desc_t& member);
    virtual bool do_expected_wire(const std::vector<uint8_t>& wire, int& errorcode);

    // The shared test literal for a native type, cast or suffixed to suit the
    // member's Java type (e.g. "(byte) -50", "3000000000L", "3.5f").
    std::string java_literal(const std::string& type);
};

}
