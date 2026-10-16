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
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>
#include "errorcodes.h"
#include "types.h"
#include "specification/specification.h"

namespace skyloupe 
{

class Generator
{
public:
    enum member_index { member_type = 0, member_name = 1, member_endianness = 2, member_length = 3 };
    typedef std::tuple<std::string, std::string, unsigned int, unsigned int> member_desc_t;

    Generator() = delete;
    Generator(Specification& msg_specification, Specification& lang_specification);
    virtual ~Generator() = default;

    virtual bool generate(int& errorcode) = 0;
    virtual bool is_native_type(const std::string& type);
    virtual bool is_string_type(const std::string& type);
    // A member type that is neither native nor a string names another
    // generated message, in the same namespace.
    bool is_user_type(const std::string& type);
    virtual results_t get_results() const { return results_; }  //copy???

    // The specs of the nested message types this message uses (and those
    // they use), by type name, for generators that look inside nested
    // members: the relays.
    void set_nested_specs(const std::map<std::string, Specification>& specs);

protected:
    Specification msg_spec_;
    Specification lang_spec_;
    results_t results_;
    std::stringstream ss_;
    int errorcode_ = 0;
    std::map< uint32_t, member_desc_t> ordered_members_;

    void cr();
    void insert_char(const char c);
    bool insert_indents(unsigned int indents = 0);
    bool gen_one_line(const std::string& key, unsigned int indents = 0);
    bool gen_one_line_subst(const std::string& template_key, const std::string& subst_key, const std::string& target_key, unsigned int indents);
    bool inorder_members(int& errorcode, unsigned int indents = 0);
    bool inorder_functions(const std::string& funct_str, int& errorcode, unsigned int indents = 0, bool prepend_classname = false);
    bool inorder_member_names(int& errorcode, unsigned int indents = 0);
    // The distinct nested message types this message uses, sorted.
    std::set<std::string> user_types();
    // C and C++ includes: the given ones, then each nested type's generated
    // header ("T.h") that isn't already among them.
    std::vector<std::string> with_user_headers(std::vector<std::string> includes);

    std::set<std::string> native_types_;

    typedef std::map<uint32_t, member_desc_t> members_t;
    typedef std::function<void(const std::vector<std::string>& path, const member_desc_t& member)> leaf_visitor_t;

    static members_t members_of(Specification& spec);

    // Nested types' members, from set_nested_specs.
    std::map<std::string, members_t> nested_members_;

    // Visits every native and string member in wire order, descending into
    // nested messages; path holds the member names from this message down
    // to the visited one. Returns false, after naming it on stderr, if a
    // nested type's spec is missing.
    bool for_each_leaf(const leaf_visitor_t& visit);
    bool for_each_leaf(const members_t& members, std::vector<std::string>& path, const leaf_visitor_t& visit);

    // The nested types this message uses, directly or not, each after the
    // types it contains.
    std::vector<std::string> nested_types_in_order();

    // A native or string member, reached through path (member names from
    // this message down).
    struct leaf_t
    {
        std::vector<std::string> path;
        member_desc_t member;

        // The path's names, each prefixed by before and followed by after,
        // joined by between (e.g. "got" + ".Get_a()" + ".Get_b()").
        std::string join(const std::string& between, const std::string& before = "", const std::string& after = "") const;
        // The members containing this one, each as before + name + after.
        std::string join_parents(const std::string& before, const std::string& after) const;
    };

    // Every member the relays check, nested ones included; each must have a
    // shared test value. Returns false (setting errorcode) otherwise.
    bool relay_leaves(std::vector<leaf_t>& leaves, int& errorcode);
};

}
