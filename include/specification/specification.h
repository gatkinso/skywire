//
//   Copyright (C) 2017 Skyloupe, LLC
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

#include <vector>
#include <string>
#pragma warning(disable:4800)
#include "specification.pb.h"
#pragma warning(default:4800)

namespace skyloupe
{

class Specification
{
public:
    Specification() { };

    virtual bool read_file(const std::string& pathname);
    virtual bool write_file(const std::string& pathname) const;

    virtual bool read_string(const std::string& json_str);
    virtual bool write_string(std::string& json_str) const;

    virtual std::vector<std::string> get_value(const::std::string& key, const std::string nomatch = "@NOTFOUND");
    virtual std::string get_value_string(const::std::string& key, const std::string nomatch = "@NOTFOUND");
    virtual bool get_value_member(uint32_t key, std::string& type_val, std::string& name_val, uint32_t& endian);

    virtual bool set_value(const::std::string& key, const std::string& value);
    virtual bool set_value_string(const::std::string& key, const std::string& value);
    virtual bool set_value_member(uint32_t key, const std::string& type, const std::string& name, uint32_t endian_val);

    virtual bool clear();
    virtual bool clear_key(const::std::string& key);
    virtual bool clear_key_string(const::std::string& key);
    virtual bool clear_key_member(uint32_t key);

    const skyloupe::skywireproto::Specification& get_proto() const { return spec_proto_; }
    skyloupe::skywireproto::Specification& get_proto_ref() { return spec_proto_; }
    void set_proto(const skyloupe::skywireproto::Specification& settings) { spec_proto_.CopyFrom(settings); }

private:
    skyloupe::skywireproto::Specification spec_proto_;
};

}



