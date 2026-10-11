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
#include "errorcodes.h"
#include "types.h"

namespace skyloupe
{
    // language may be empty; otherwise the lang file must be for that language.
    // name_space, when not empty, replaces the specification's namespace.
    bool process_file(const std::string& pathname, results_t& output_str_map, const std::string& outdirname, const std::string& langfile, const std::string& language, int& errorcode, bool generate_tests = true, bool write_to_file = false, bool generate_relay = false, const std::string& name_space = std::string());

    bool process_string(const std::string& input_msg_str, results_t& output_str_map, const std::string& langstring, const std::string& language, const std::string& outdirname, int& errorcode, bool generate_tests = true, bool write_to_file = false, bool generate_relay = false, const std::string& name_space = std::string());

    // Looks up a language name (or alias such as "cpp") for the language its
    // lang file declares and that lang file's name, e.g. "cpp" -> "c++", "lang.cpp.json".
    bool find_language(const std::string& name, std::string& language, std::string& lang_filename);

    // The accepted language names, for messages.
    std::string language_names();

    // The skywire version, e.g. "0.9.0".
    const char* version();
}