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

class Py3Generator : public Generator
{
public:
    Py3Generator() = delete;
    Py3Generator(Specification& specification) : Generator(specification) {}
    virtual ~Py3Generator() {}

    bool generate(int& errorcode) override;

protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_sentry(int& errorcode);
    virtual bool imp_includes(int& errorcode);
    virtual bool imp_namespace(int& errorcode);
    virtual bool imp_classname(int& errorcode);
    virtual bool imp_public(int& errorcode);
    virtual bool imp_private(int& errorcode);
    virtual bool imp_cons(int& errorcode);
    virtual bool imp_des(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_equals(int& errorcode);
    virtual bool imp_print(int& errorcode);
};

}
