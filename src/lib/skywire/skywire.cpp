//
//   Copyright (C) 2017 Geoffrey Atkinson
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

#include "skywire.h"
#include "processor.h"
#include "skywire_version.h"

namespace skyloupe
{

bool process_file(const std::string& pathname, results_t& output_str_map, const std::string& outdirname, const std::string& langfile, const std::string& language, int& errorcode, bool generate_tests, bool write_to_file, bool generate_relay, const std::string& name_space)
{
    skyloupe::Processor processor;
    processor.set_language(language);
    processor.set_namespace(name_space);

    bool success = processor.process_file(pathname, langfile, output_str_map, outdirname, errorcode, generate_tests, write_to_file, generate_relay);

    return success;
}

bool process_string(const std::string& input_str, results_t& output_str_map, const std::string& langstring, const std::string& language, const std::string& outputdir, int& errorcode, bool generate_tests, bool write_to_file, bool generate_relay, const std::string& name_space)
{
    skyloupe::Processor processor;
    processor.set_language(language);
    processor.set_namespace(name_space);

    bool success = processor.process_string(input_str, langstring, output_str_map, outputdir, errorcode, generate_tests, write_to_file, generate_relay);

    return success;
}

namespace
{

struct LanguageName
{
    const char* name;
    const char* language;
    const char* lang_filename;
};

const LanguageName language_table[] = {
    { "c",       "c",       "lang.c.json" },
    { "c++",     "c++",     "lang.cpp.json" },
    { "cpp",     "c++",     "lang.cpp.json" },
    { "go",      "go",      "lang.go.json" },
    { "java",    "java",    "lang.java.json" },
    { "proto3",  "proto3",  "lang.proto3.json" },
    { "python3", "python3", "lang.python3.json" },
    { "python",  "python3", "lang.python3.json" },
    { "rust",    "rust",    "lang.rust.json" },
    { "swift",   "swift",   "lang.swift.json" },
};

}

const char* version()
{
    return SKYWIRE_VERSION;
}

int version_major()
{
    return SKYWIRE_VERSION_MAJOR;
}

int version_minor()
{
    return SKYWIRE_VERSION_MINOR;
}

int version_patch()
{
    return SKYWIRE_VERSION_PATCH;
}

const char* revision()
{
    return SKYWIRE_REVISION;
}

const build_info_t& build_info()
{
    static const build_info_t info = {
        SKYWIRE_COMMIT,
        SKYWIRE_COMMIT_DATE,
        SKYWIRE_BRANCH,
        SKYWIRE_COMPILER,
        SKYWIRE_BUILD_TYPE,
        SKYWIRE_PLATFORM,
        SKYWIRE_BUILD_OPTIONS,
    };
    return info;
}

bool find_language(const std::string& name, std::string& language, std::string& lang_filename)
{
    for (const auto& entry : language_table)
    {
        if (name == entry.name)
        {
            language = entry.language;
            lang_filename = entry.lang_filename;
            return true;
        }
    }

    return false;
}

std::string language_names()
{
    std::string names;
    for (const auto& entry : language_table)
    {
        names += (names.empty() ? "" : ", ") + std::string(entry.name);
    }
    return names;
}

}
