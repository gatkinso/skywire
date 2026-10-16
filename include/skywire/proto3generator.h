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
#include "generator.h"

namespace skyloupe 
{

class Proto3Generator : public Generator
{
public:
    Proto3Generator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~Proto3Generator() {}

    bool generate(int& errorcode) override;

 protected:
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_syntax(int& errorcode);
    virtual bool imp_package(int& errorcode);
    virtual bool imp_imports(int& errorcode);
    virtual bool imp_message(int& errorcode);

    std::string proto_type(const std::string& type);
    std::string skywire_comment(const member_desc_t& member);

private:
    std::string classname_;

    std::string subst(std::string template_str, uint32_t tag, const member_desc_t& member);
};

}
