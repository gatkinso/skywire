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
#include "stencil.pb.h"

namespace skyloupe
{

class Stencil
{
public:
    Stencil() { };

    bool read_file(const std::string& pathname);
    bool write_file(const std::string& pathname) const;

    std::vector<std::string> get_value(const::std::string& key, const std::string nomatch = "@NOTFOUND");
    std::string get_value_string(const::std::string& key, const std::string nomatch = "@NOTFOUND");
    bool get_value_int(const::std::string& key, int& value, int nomatch = 0);
    bool get_value_bool(const::std::string& key, bool& value, bool = false);
    bool get_value_member(uint32_t key, std::string& type_val, std::string& name_val);

    bool set_value(const::std::string& key, const std::string& value);
    bool set_value_string(const::std::string& key, const std::string& value);
    bool set_value_int(const::std::string& key, int value);
    bool set_value_bool(const::std::string& key, bool value);
    bool set_value_member(uint32_t key, const std::string& type, const std::string& name);

    bool clear();
    bool clear_key(const::std::string& key);
    bool clear_key_string(const::std::string& key);
    bool clear_key_int(const::std::string& key);
    bool clear_key_bool(const::std::string& key);
    bool clear_key_member(uint32_t key);

    const skyloupe::skywireproto::Stencil& get_proto() const { return stencil_proto_; }
    void set_proto(const skyloupe::skywireproto::Stencil& settings) { stencil_proto_.CopyFrom(settings); }

private:
    skyloupe::skywireproto::Stencil stencil_proto_;
};

}



