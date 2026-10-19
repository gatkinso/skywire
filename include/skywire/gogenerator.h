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
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_package(int& errorcode);
    virtual bool imp_imports(int& errorcode);
    virtual bool imp_struct(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_equals(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);

    std::string go_type(const std::string& type);
    std::string go_endian(unsigned int endian_flag);
    unsigned int go_bits(const std::string& type);
    bool is_float_type(const std::string& type);

private:
    std::string classname_;

    bool do_block(const std::string block_id, std::function<std::string()> builder);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string members_at(const std::string& direction);
    std::string serialize_bitfield(uint32_t tag, const member_desc_t& member, const std::string& index, const std::string& at);
    std::string deserialize_bitfield(uint32_t tag, const member_desc_t& member, const std::string& index, const std::string& at);
};

}
