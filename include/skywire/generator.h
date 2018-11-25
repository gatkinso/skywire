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
#include <vector>
#include <sstream>
#include "errorcodes.h"
#include "stencil/stencil.h"

namespace skyloupe 
{

class Generator
{
public:
    typedef std::vector<std::string> results_t;

    Generator() = delete;
    Generator(Stencil& stencil) : msg_stencil_(stencil) {}
    virtual ~Generator() = default;

    virtual bool generate(int& errorcode) = 0;
    virtual bool set_lang_settings(const std::string pathname);
    virtual bool set_msg_settings(const std::string pathname);

    virtual results_t get_results() const { return results_; }

protected:
    Stencil lang_stencil_;
    Stencil msg_stencil_;
    results_t results_;
    std::stringstream ss_;
    int errorcode_;

    void cr();
    void insert_char(const char c);
    bool insert_indents(unsigned int indents = 0);
    bool gen_one_line(const std::string& key, unsigned int indents = 0);
    bool gen_one_line_subst(const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents);
    bool inorder_members(int& errorcode, unsigned int indents = 0);
    bool inorder_functions(const std::string& funct_str, int& errorcode, unsigned int indents = 0, bool prepend_classname = false);
};

}
