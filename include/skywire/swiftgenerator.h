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

class SwiftGenerator : public Generator
{
public:
    SwiftGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~SwiftGenerator() {}

    bool generate(int& errorcode) override;

private:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_base(int& errorcode);

private:
    std::string classname_;
};

}
