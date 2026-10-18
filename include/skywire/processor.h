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
#include <map>
#include "specification/specification.h"
#include "generators.h"
#include "errorcodes.h"
#include "types.h"

namespace skyloupe 
{

class Processor
{
public:
    Processor() {}
    virtual ~Processor() {}

    virtual bool process_string(const std::string& input_msg_str, const std::string& input_lang_str, results_t& output_str_map, const std::string& outputdir, int& errorcode, bool generate_tests = true, bool write_to_file = false, bool generate_relay = false);
    virtual bool process_file(const std::string& msgpathname, const std::string& langpathname, results_t& output_str_map, const std::string& outputdir, int& errorcode, bool generate_tests = true, bool write_to_file = false, bool generate_relay = false);
    // The target language is the one the lang file names; when set, it must be this one.
    void set_language(const std::string& language) { expected_language_ = language; }
    // When set, replaces the specification's namespace.
    void set_namespace(const std::string& name_space) { namespace_ = name_space; }

private:
    Specification msg_spec_;
    Specification lang_spec_;

    std::string language_key_ = "language";
    std::string expected_language_;
    std::string namespace_;
    // The message spec's directory, where a relay finds nested messages'
    // specs ("<type>.json"); empty for process_string.
    std::string spec_dir_;

    std::string swift_str_ = "swift";
    std::string c_str_ = "c";
    std::string cpp_str_ = "c++";
    std::string go_str_ = "go";
    std::string java_str_ = "java";
    std::string proto3_str_ = "proto3";
    std::string py3_str_ = "python3";
    std::string rust_str_ = "rust";

    virtual bool process(results_t& output_str_map, int& errorcode, bool generate_tests, bool generate_relay);
    virtual void resolve_output_filenames();
    virtual void resolve_includes();
    virtual bool validate_spec(int& errorcode);
    bool load_nested_specs(Specification& spec, std::map<std::string, Specification>& specs, std::vector<std::string>& stack, int& errorcode);
    virtual bool write_results(const std::string& outputdir, const results_t& output_str_map, int& errorcode);
};

}
