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

#include <string>
#include "generator.h"

namespace skyloupe 
{

class CppTestGenerator : public Generator
{
public:
    CppTestGenerator() = delete;
    CppTestGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~CppTestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);

    virtual bool test_banner(int& errorcode);
    virtual bool test_includes(int& errorcode);
    virtual bool test_serdes(int& errorcode);
    virtual bool test_size(int& errorcode);
    virtual bool test_equality(int& errorcode);
    virtual bool test_short_buffer(int& errorcode);
    virtual bool test_wire(int& errorcode);
    virtual bool test_bitfield_setters(int& errorcode);
    virtual bool test_pz_round_trip(int& errorcode);
    virtual bool test_pz_wire(int& errorcode);
    virtual bool test_pz_empty(int& errorcode);
    virtual bool test_pz_short_buffer(int& errorcode);

    // The JSON or XML tests (format "json" or "xml"), when asked for.
    virtual bool test_text(const std::string& format, int& errorcode);
    // The BSON tests, when asked for.
    virtual bool test_bson(int& errorcode);

    virtual bool do_sets(int& errorcode);

};

}
