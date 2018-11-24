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
    CppGenerator(Stencil& stencil);
    virtual ~CppGenerator() = default;

    virtual bool generate(int& errorcode);

protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool hdr_banner(std::string& str, int& errorcode);
    virtual bool hdr_sentry(std::string& str, int& errorcode);
    virtual bool hdr_includes(std::string& str, int& errorcode);
    virtual bool hdr_namespace(std::string& str, int& errorcode);
    virtual bool hdr_classname(std::string& str, int& errorcode);
    virtual bool hdr_public(std::string& str, int& errorcode);
    virtual bool hdr_private(std::string& str, int& errorcode);
    virtual bool hdr_cons(std::string& str, int& errorcode);
    virtual bool hdr_des(std::string& str, int& errorcode);
    virtual bool hdr_get_functions(std::string& str, int& errorcode);
    virtual bool hdr_set_functions(std::string& str, int& errorcode);
    virtual bool hdr_serialize(std::string& str, int& errorcode);
    virtual bool hdr_deserialize(std::string& str, int& errorcode);
    virtual bool hdr_size(std::string& str, int& errorcode);
    virtual bool hdr_members(std::string& str, int& errorcode);
    virtual bool hdr_classend(std::string& str, int& errorcode);
    virtual bool hdr_namespaceend(std::string& str, int& errorcode);

private:
    bool inorder_functions(std::string& str, const std::string& funct_str, int& errorcode);
    bool inorder_members(std::string& str, int& errorcode);
};

}
