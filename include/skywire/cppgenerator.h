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

class CppGenerator : public Generator
{
public:
    CppGenerator() = delete;
    CppGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~CppGenerator() = default;

    bool generate(int& errorcode) override;

protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool hdr_banner(int& errorcode);
    virtual bool hdr_sentry(int& errorcode);
    virtual bool hdr_includes(int& errorcode);
    virtual bool hdr_namespace(int& errorcode);
    virtual bool hdr_classname(int& errorcode);
    virtual bool hdr_public(int& errorcode);
    virtual bool hdr_private(int& errorcode);
    virtual bool hdr_cons(int& errorcode);
    virtual bool hdr_des(int& errorcode);
    virtual bool hdr_get_functions(int& errorcode);
    virtual bool hdr_set_functions(int& errorcode);
    virtual bool hdr_serialize(int& errorcode);
    virtual bool hdr_deserialize(int& errorcode);
    virtual bool hdr_size(int& errorcode);
    virtual bool hdr_equals(int& errorcode);
    virtual bool hdr_members(int& errorcode);
    virtual bool hdr_classend(int& errorcode);
    virtual bool hdr_namespaceend(int& errorcode);
};

}
