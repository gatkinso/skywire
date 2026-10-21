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

// Writes a protobuf text-format message holding the shared test values, headed
// by the canonical proto3 bytes protoc must produce from it (see
// proto3_roundtrip.cmake).
class Proto3TestGenerator : public Generator
{
public:
    Proto3TestGenerator() = delete;
    Proto3TestGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~Proto3TestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);
};

}
