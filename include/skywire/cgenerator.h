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

class CGenerator : public Generator
{
public:
    CGenerator() = delete;
    CGenerator(Stencil& stencil) : Generator(stencil) {}
    virtual ~CGenerator() {}

    virtual bool generate(int& errorcode);

protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool hdr_banner(std::string& str, int& errorcode);
    virtual bool hdr_sentry_top(std::string& str, int& errorcode);
    virtual bool hdr_includes(std::string& str, int& errorcode);
    virtual bool hdr_extern_top(std::string& str, int& errorcode);
    virtual bool hdr_structtag(std::string& str, int& errorcode);
    virtual bool hdr_members(std::string& str, int& errorcode);
    virtual bool hdr_structtype(std::string& str, int& errorcode);
    virtual bool hdr_serialize(std::string& str, int& errorcode);
    virtual bool hdr_deserialize(std::string& str, int& errorcode);
    virtual bool hdr_size(std::string& str, int& errorcode);
    virtual bool hdr_get_functions(std::string& str, int& errorcode);
    virtual bool hdr_set_functions(std::string& str, int& errorcode);
    virtual bool hdr_equals_function(std::string& str, int& errorcode);
    virtual bool hdr_extern_bottom(std::string& str, int& errorcode);
    virtual bool hdr_sentry_bottom(std::string& str, int& errorcode);

    virtual bool imp_banner(std::string& str, int& errorcode);
    virtual bool imp_includes(std::string& str, int& errorcode);
    virtual bool imp_serialize(std::string& str, int& errorcode);
    virtual bool imp_deserialize(std::string& str, int& errorcode);
    virtual bool imp_size(std::string& str, int& errorcode);
    virtual bool imp_get_functions(std::string& str, int& errorcode);
    virtual bool imp_set_functions(std::string& str, int& errorcode);
    virtual bool imp_equals_function(std::string& str, int& errorcode);
};

}
