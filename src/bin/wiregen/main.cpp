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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#include "argv.h"
#include "skywire/skywire.h"

using namespace skyloupe;

qwikpirate::ArgVee cmdargs;
std::string config_pathname = "settings.json";
std::string input_filename;
std::string input_str;
std::string output_directory = ".";
results_t output_str_map;
std::string language;
std::string lang_file;
std::string langspec;
std::string name_space;
bool silent = false;
bool generate_tests = false;
bool generate_relay = false;
bool file_mode = false;
bool write_to_file = false;

void cli_callback(const std::string& flag, const std::string& value, void* param)
{
    if (flag.compare("--help") == 0)
    {
        std::cout << "Use: wiregen  (no paramters)               -- defaults to using settings.json" << std::endl
                  << "     wiregen --version                     -- prints version and exits" << std::endl
                  << "     wiregen --config filename             -- launches using fiename for configuration file" << std::endl
                  << "     wiregen --input_file filename         -- launches using filename for input" << std::endl
                  << "     wiregen --input_str json_msg_spec     -- launches using provided json string for input" << std::endl
                  << "     wiregen --outdir directoryname        -- directory to write generated file(s) to" << std::endl
                  << "     wiregen --language name               -- language to generate: " << language_names() << std::endl
                  << "                                              (uses the lang file installed next to wiregen)" << std::endl
                  << "     wiregen --lang_file filename          -- lang file to use instead of the installed one" << std::endl
                  << "     wiregen --langspec string             -- lang file contents to use instead of a file" << std::endl
                  << "     wiregen --namespace name              -- namespace (package) to generate into, replacing the spec's" << std::endl
                  << "     wiregen --generate_tests              -- generate unit tests" << std::endl
                  << "     wiregen --generate_relay              -- generate a relay program (cross-language test)" << std::endl
                  << "     wiregen --silent                      -- no output displayed" << std::endl
                  << "     wiregen --write_to_file               -- write output to file (if absent outputs to stdout)" << std::endl;
        exit(0);
    }

    if (flag.compare("--version") == 0)
    {
        std::cout << "TODO not yet implemented" << std::endl;
        exit(0);
    }

    if (flag.compare("--config") == 0)
    {
        if (false == value.empty())
        {
            config_pathname = value;
        }
        else
        {
            std::cout << "Need a file name for --config switch" << std::endl;
        }
    }

    if (flag.compare("--input_file") == 0)
    {
        file_mode = true;
        if (false == value.empty())
        {
            input_filename = value;
        }
        else
        {
            std::cout << "Need a file name for --input_file switch" << std::endl;
        }
    }

    if (flag.compare("--input_str") == 0)
    {
        if (file_mode)
        {
            std::cout << "Cannot mix file and string input modes. Exiting." << std::endl;
            exit(1);
        }

        if (false == value.empty())
        {
            input_str = value;
        }
        else
        {
            std::cout << "Need a message for --input_string switch" << std::endl;
        }
    }

    if (flag.compare("--outdir") == 0)
    {
        if (false == value.empty())
        {
            output_directory = value;
        }
        else
        {
            std::cout << "Need a valid output directory name" << std::endl;
        }
    }

    if (flag.compare("--language") == 0)
    {
        if (false == value.empty())
        {
            language = value;
        }
        else
        {
            std::cout << "Need a language name for --language switch" << std::endl;
        }
    }

    // --langfile is the old spelling of --lang_file.
    if (flag.compare("--lang_file") == 0 || flag.compare("--langfile") == 0)
    {
        if (false == value.empty())
        {
            lang_file = value;
        }
        else
        {
            std::cout << "Need a file name for " << flag << " switch" << std::endl;
        }
    }

    if (flag.compare("--langspec") == 0)
    {
        if (false == value.empty())
        {
            langspec = value;
        }
    }

    if (flag.compare("--namespace") == 0)
    {
        if (false == value.empty())
        {
            name_space = value;
        }
        else
        {
            std::cout << "Need a name for --namespace switch" << std::endl;
        }
    }

    if (flag.compare("--silent") == 0)
    {
        if (false != value.empty())
        {
            silent = true;
        }
    }

    if (flag.compare("--generate_tests") == 0)
    {
        if (false != value.empty())
        {
            generate_tests = true;
        }
    }

    if (flag.compare("--generate_relay") == 0)
    {
        if (false != value.empty())
        {
            generate_relay = true;
        }
    }

    if (flag.compare("--write_to_file") == 0)
    {
        if (false != value.empty())
        {
            write_to_file = true;
        }
    }    
}

bool process_cmd_line(int argc, const char* argv[])
{
    std::vector<std::string> cli_errors_;

    cmdargs.setOption("--help", true, true, cli_callback);
    cmdargs.setOption("--version", true, true, cli_callback);
    cmdargs.setOption("--config", true, false, cli_callback);
    cmdargs.setOption("--input_file", true, false, cli_callback);
    cmdargs.setOption("--input_str", true, false, cli_callback);
    cmdargs.setOption("--outdir", true, false, cli_callback);
    cmdargs.setOption("--language", true, false, cli_callback);
    cmdargs.setOption("--lang_file", true, false, cli_callback);
    cmdargs.setOption("--langfile", true, false, cli_callback);
    cmdargs.setOption("--langspec", true, false, cli_callback);
    cmdargs.setOption("--namespace", true, false, cli_callback);
    cmdargs.setOption("--generate_tests", true, true, cli_callback);
    cmdargs.setOption("--generate_relay", true, true, cli_callback);
    cmdargs.setOption("--silent", true, true, cli_callback);
    cmdargs.setOption("--write_to_file", true, true, cli_callback);

    bool ret = cmdargs.parse(argc, argv, cli_errors_);

    cmdargs.apply();

    return ret;
}

// The directory wiregen runs from; the build and the install put the lang files there.
std::filesystem::path executable_dir(const char* argv0)
{
    namespace fs = std::filesystem;
    std::error_code ec;
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
    {
        return fs::path(buf).parent_path();
    }
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buf(size, '\0');
    if (_NSGetExecutablePath(buf.data(), &size) == 0)
    {
        fs::path exe = fs::canonical(buf.c_str(), ec);
        if (!ec)
        {
            return exe.parent_path();
        }
    }
#else
    fs::path exe = fs::read_symlink("/proc/self/exe", ec);
    if (!ec)
    {
        return exe.parent_path();
    }
#endif
    fs::path from_argv = fs::absolute(argv0, ec);
    return ec ? fs::path(".") : from_argv.parent_path();
}

bool read_text_file(const std::string& pathname, std::string& text)
{
    std::ifstream in(pathname, std::ios::binary);
    if (!in)
    {
        return false;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    text = ss.str();
    return true;
}

int main(int argc, const char* argv[])
{
    bool success = false;
    int errorcode = -1;

    success = process_cmd_line(argc, argv);
    if (false == success)
    {
        std::cout << "Invalid input parameters" << std::endl;
        return 1;
    }

    // --language picks the installed lang file; --lang_file or --langspec replaces it.
    std::string lang_name;
    if (false == language.empty())
    {
        std::string installed_lang_file;
        if (false == find_language(language, lang_name, installed_lang_file))
        {
            std::cout << "Unknown language: " << language << " (use one of: " << language_names() << ")" << std::endl;
            return SW_UNKNOWNLANG;
        }

        if (lang_file.empty() && langspec.empty())
        {
            lang_file = (executable_dir(argv[0]) / installed_lang_file).string();
        }
    }

    if (lang_file.empty() && (file_mode || langspec.empty()))
    {
        std::cout << "Choose a language with --language (" << language_names() << ") or give a --lang_file" << std::endl;
        return 1;
    }

    if (true == file_mode)
    {
        success = process_file(input_filename, output_str_map, output_directory, lang_file, lang_name, errorcode, generate_tests, write_to_file, generate_relay, name_space);
    } 
    else
    {
        if (langspec.empty() && false == read_text_file(lang_file, langspec))
        {
            std::cout << "Cannot read lang file: " << lang_file << std::endl;
            return SW_FILEREADERROR;
        }

        success = process_string(input_str, output_str_map, langspec, lang_name, output_directory, errorcode, generate_tests, write_to_file, generate_relay, name_space);
    }

    if (false == write_to_file)
    {
        for (auto item : output_str_map)
        {
            std::cout << item.second << std::endl;
        }
    }

    if (success == true)
    {
        if(!silent) 
            std::cout << "Successful process: " << input_filename << std::endl;
    }
    else
    {
        if (!silent)
        {
            std::cout << "Failure processing: " << input_filename << std::endl;
            std::cout << "Error number: " << errorcode << std::endl;
        }
    }

    // Exit status is only 8 bits; never let a failure truncate to 0.
    if (success == true)
    {
        return 0;
    }

    return (errorcode & 0xFF) != 0 ? (errorcode & 0xFF) : 1;
}

//bin/wiregen --language c++ --generate_tests --input_str "{\"stringValues\":{\"classname\":\"Classy\",\"outputheaderfilename\":\"Classy.h\",\"outputtestfilename\":\"ClassyTest.cpp\",\"namespace\":\"testy\"},\"members\":{\"100\":{\"type\":\"float\",\"name\":\"a\"},\"200\":{\"type\":\"uint32_t\",\"name\":\"b\"}},\"includes\":[\"<streamable.h>\",\"<stdint.h>\"]}"
