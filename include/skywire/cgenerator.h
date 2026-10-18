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
    CGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~CGenerator() {}

    bool generate(int& errorcode) override;

protected:
    virtual bool do_header(int& errorcode);
    virtual bool do_implementation(int& errorcode);

    virtual bool hdr_banner(int& errorcode);
    virtual bool hdr_sentry_top(int& errorcode);
    virtual bool hdr_includes(int& errorcode);
    virtual bool hdr_extern_top(int& errorcode);
    virtual bool hdr_structtag(int& errorcode);
    virtual bool hdr_members(int& errorcode);
    virtual bool hdr_structtype(int& errorcode);
    virtual bool hdr_serialize(int& errorcode);
    virtual bool hdr_deserialize(int& errorcode);
    virtual bool hdr_size(int& errorcode);
    virtual bool hdr_get_functions(int& errorcode);
    virtual bool hdr_set_functions(int& errorcode);
    virtual bool hdr_equals_function(int& errorcode);
    virtual bool hdr_extern_bottom(int& errorcode);
    virtual bool hdr_sentry_bottom(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_includes(int& errorcode);
    virtual bool imp_serialize(int& errorcode);
    virtual bool imp_deserialize(int& errorcode);
    virtual bool imp_size(int& errorcode);
    virtual bool imp_equals_function(int& errorcode);
    virtual bool imp_get_functions(int& errorcode);
    virtual bool imp_set_functions(int& errorcode);

private:
    virtual bool getset_functions(const std::string& fcn_key, const std::string& ass_key, int& errorcode);
    static std::string endian_flag(unsigned int endian);
    void write_bitfield(uint32_t tag, const member_desc_t& member);
    void read_bitfield(uint32_t tag, const member_desc_t& member);
};

}
