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
#include <map>
#include <sstream>
#include <tuple>
#include "errorcodes.h"
#include "types.h"
#include "stencil/stencil.h"

namespace skyloupe 
{

class Generator
{
public:
    static const std::multimap<std::string, std::string> GeneratorTypes;
    enum member_index { member_type = 0, member_name = 1, member_endianness = 2 };
    typedef std::tuple<std::string, std::string, unsigned int> member_desc_t;

    Generator() = delete;
    Generator(Stencil& stencil);
    virtual ~Generator() = default;

    virtual bool generate(int& errorcode) = 0;
    virtual bool is_native_type(const std::string& type);
    virtual bool set_lang_settings(const std::string pathname);
    virtual bool set_msg_settings(const std::string pathname);
    virtual results_t get_results() const { return results_; }  //copy???

protected:
    Stencil lang_stencil_;
    Stencil msg_stencil_;
    results_t results_;
    std::stringstream ss_;
    int errorcode_;
    std::map< uint32_t, member_desc_t> ordered_members_;

    void cr();
    void insert_char(const char c);
    bool insert_indents(unsigned int indents = 0);
    bool gen_one_line(const std::string& key, unsigned int indents = 0);
    bool gen_one_line_subst(const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents);
    bool inorder_members(int& errorcode, unsigned int indents = 0);
    bool inorder_functions(const std::string& funct_str, int& errorcode, unsigned int indents = 0, bool prepend_classname = false);

    std::set<std::string> native_types_;
};

}
