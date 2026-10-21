//
//   Copyright (C) 2018 Geoffrey Atkinson
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

class JavaGenerator : public Generator
{
public:
    JavaGenerator(Specification& mspec, Specification& lspec) : Generator(mspec, lspec) {}
    virtual ~JavaGenerator() {}

    bool generate(int& errorcode) override;

    // How a native member travels through a ByteBuffer. Java has no unsigned
    // types, so the Java type can be wider than the wire type.
    struct wire_t
    {
        std::string type;       // Java type on the wire: byte, short, int, long, float, double
        std::string accessor;   // ByteBuffer get/put suffix: "", Short, Int, Long, Float, Double
        unsigned int bits;
    };

    // Wire type of a native member type; bits is 0 for strings and user types.
    static wire_t java_wire(const std::string& type);

protected:
    virtual bool do_implementation(int& errorcode);

    virtual bool imp_banner(int& errorcode);
    virtual bool imp_package(int& errorcode);
    virtual bool imp_imports(int& errorcode);
    virtual bool imp_class(int& errorcode);

    std::string imp_fields();
    std::string imp_serialize();
    std::string imp_deserialize();
    std::string imp_size();
    std::string imp_equals();
    std::string imp_hashcode();
    std::string imp_tostring();
    std::string imp_get_set_functions();

    std::string java_type(const std::string& type);
    std::string java_endian(unsigned int endian_flag);
    bool is_widened(const std::string& type);

private:
    std::string classname_;

    std::string block(const std::string block_id, const std::string& body);
    std::string subst(std::string template_str, const member_desc_t& member);
    std::string serialize_bitfield(uint32_t tag, const member_desc_t& member);
    std::string deserialize_bitfield(uint32_t tag, const member_desc_t& member);
    std::string set_bitfield(const member_desc_t& member);
};

}
