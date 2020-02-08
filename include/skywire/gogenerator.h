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

#include <string>
#include <functional>
#include "generator.h"

namespace skyloupe 
{

class GoGenerator : public Generator
{
public:
    GoGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~GoGenerator() {}

    bool generate(int& errorcode) override;

 protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_b0(int& errorcode);
    virtual bool imp_b1(int& errorcode);
    virtual bool imp_b2(int& errorcode);
    virtual bool imp_b3(int& errorcode);
    virtual bool imp_b4(int& errorcode);
    virtual bool imp_b5(int& errorcode);
    virtual bool imp_b6(int& errorcode);
    virtual bool imp_b7(int& errorcode);
    virtual bool imp_b8(int& errorcode);

private:
    std::string classname_;

    bool do_block(const std::string block_id, std::function<std::string()> builder);
};

}
