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

#include <map>
#include <string>
#include "stencil/stencil.h"
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

    virtual bool process_string(const std::string& input_str, results_t& output_str_map, int& errorcode, bool generate_tests = true);
    virtual bool process_file(const std::string& pathname, const std::string& outputdir, int& errorcode, bool generate_tests = true);
    void set_langfile(const std::string& filename) { language_file_ = filename; }
private:
    Stencil msg_stencil_;

    std::string language_key_ = "language";
    std::string language_file_key_ = "lang_file";
    std::string language_file_;

    std::string ada_str_ = "ada";
    std::string c_str_ = "c";
    std::string cpp_str_ = "c++";
    std::string go_str_ = "go";
    std::string java_str_ = "java";
    std::string py3_str_ = "python3";

    virtual bool process(results_t& output_str_map, int& errorcode, bool generate_tests);
    virtual void do_file_write(const std::string& outputdir, results_t& output_str_map, const std::string stencil_key, const std::string value_key, int& errorcode);
};

}
