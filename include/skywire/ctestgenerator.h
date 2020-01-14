//
//   Copyright (C) 2018 Skyloupe, LLC
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

class CTestGenerator : public Generator
{
public:
    CTestGenerator() = delete;
    CTestGenerator(Specification& spec);
    virtual ~CTestGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_test_implementation(int& errorcode);

    virtual bool test_banner(int& errorcode);
    virtual bool test_includes(int& errorcode);
    virtual bool test_serdes(int& errorcode);
    virtual bool test_size(int& errorcode);
    virtual bool test_equality(int& errorcode);

    virtual bool do_sets(int& errorcode);

private:
    char BASECHAR = 1;
    unsigned char BASEUCHAR = 2;
    uint8_t BASEU8 = 3;
    int8_t BASES8 = -4;
    uint16_t BASEU16 = 5;
    int16_t BASES16 = -6;
    uint32_t BASEU32 = 7;
    int32_t BASES32 = -8;
    uint64_t BASEU64 = 9;
    int64_t BASES64 = -10;
    float BASEFLOAT = 11.1f;
    double BASEDOUBLE = 12.2;
};

}
