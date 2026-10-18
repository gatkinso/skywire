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
    Py3Generator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~Py3Generator() {}

    bool generate(int& errorcode) override;

protected:
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_includes(int& errorcode);
    virtual bool imp_classname(int& errorcode);
    virtual bool imp_cons(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_equals(int& errorcode);
    virtual bool imp_dunders(int& errorcode);
    virtual bool imp_print(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);

    // struct format for one native member, e.g. ">I" or "16s".
    std::string py_format(const member_desc_t& member);
    // Wire size of a native member in bytes; 0 for user-defined types.
    unsigned int py_size(const member_desc_t& member);

private:
    std::string classname_;

    void method(const std::string& def_key, const std::string& doc_key = "");
    void line(const std::string& text, unsigned int indents = 2);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string unit_format(const bitfields::unit_t& unit);
    std::string py_fit(const std::string& type, unsigned int bits, const std::string& expr);
    std::string serialize_unit(uint32_t tag);
};

}
