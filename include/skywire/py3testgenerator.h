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

class Py3TestGenerator : public Generator
{
public:
    Py3TestGenerator() = delete;
    Py3TestGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~Py3TestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);
    virtual bool do_sets(int& errorcode);
    virtual bool do_expected_wire(const std::vector<uint8_t>& wire, int& errorcode);
};

}
