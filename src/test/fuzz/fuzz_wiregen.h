//
//   Copyright (C) 2026 Geoffrey Atkinson
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

// What the wiregen fuzz targets (fuzz_spec, fuzz_lang) share: the files they
// run each input against, read once at start-up.

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "fuzz_check.h"

namespace fuzz
{

inline std::string read_file(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        std::fprintf(stderr, "cannot read %s\n", path.c_str());
        std::abort();
    }
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// Every language's lang file, from the source tree.
inline std::vector<std::string> read_lang_files()
{
    std::vector<std::string> files;
    for (const char* name : { "c/lang.c.json", "cpp/lang.cpp.json", "go/lang.go.json",
             "java/lang.java.json", "proto/lang.proto3.json",
             "python/lang.python3.json", "rust/lang.rust.json",
             "swift/lang.swift.json" })
    {
        files.push_back(read_file(std::string(SKYWIRE_LANG_DIR) + "/" + name));
    }
    return files;
}

}
