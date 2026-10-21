//
//   Copyright (C) 2019 Geoffrey Atkinson
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

protected:
    // Maps a message-spec (C-style) type to its Swift type via the lang spec.
    std::string swift_type(const std::string& type);
    // Resolves a member's endian flag to a Swift Endianness literal.
    std::string swift_endian(unsigned int endian_flag);
    void serialize_bitfield(uint32_t tag, const member_desc_t& member);
    void deserialize_bitfield(uint32_t tag, const member_desc_t& member);

private:
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_imports(int& errorcode);
    virtual bool imp_classname(int& errorcode);
    virtual bool imp_members(int& errorcode);
    virtual bool imp_cons(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_equals(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);
    virtual bool imp_classend(int& errorcode);

private:
    std::string classname_;
};

}
