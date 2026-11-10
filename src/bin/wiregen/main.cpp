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

#include <algorithm>
#include <filesystem>
#include <initializer_list>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
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
bool generate_json = false;
bool generate_xml = false;
bool generate_bson = false;
bool file_mode = false;
bool write_to_file = false;
bool show_banner = false;

// Joins the non-empty parts with ", ".
std::string join_present(std::initializer_list<std::string> parts)
{
    std::string joined;
    for (const auto& part : parts)
    {
        if (false == part.empty())
        {
            joined += (joined.empty() ? "" : ", ") + part;
        }
    }
    return joined;
}

void print_banner()
{
    const skyloupe::build_info_t& info = skyloupe::build_info();

    std::string title = std::string("wiregen ") + skyloupe::version();
    if (*skyloupe::revision() != '\0')
    {
        title += std::string(" (") + skyloupe::revision() + ")";
    }

    std::string commit = info.commit;
    if (false == commit.empty() && *info.commit_date != '\0')
    {
        commit += std::string(" (") + info.commit_date + ")";
    }

    // An empty line is drawn as a rule; a row whose value is empty is left out.
    const std::vector<std::pair<std::string, std::string>> rows = {
        { "", title },
        { "", "skywire message code generator" },
        { "", "" },
        { "commit:    ", commit },
        { "branch:    ", info.branch },
        { "compiler:  ", info.compiler },
        { "build:     ", join_present({ info.build_type, info.platform, info.options }) },
        { "languages: ", language_names() },
        { "", "" },
        { "", "Copyright (C) 2014-2026 Geoffrey Atkinson. Apache License 2.0." },
    };

    std::vector<std::string> lines;
    for (const auto& row : rows)
    {
        if (row.first.empty() || false == row.second.empty())
        {
            lines.push_back(row.first + row.second);
        }
    }

    size_t width = 0;
    for (const auto& line : lines)
    {
        width = std::max(width, line.size());
    }

    const std::string rule = "+" + std::string(width + 2, '-') + "+";
    std::cout << rule << std::endl;
    for (const auto& line : lines)
    {
        if (line.empty())
        {
            std::cout << rule << std::endl;
        }
        else
        {
            std::cout << "| " << line << std::string(width - line.size(), ' ') << " |" << std::endl;
        }
    }
    std::cout << rule << std::endl;
}

void print_help()
{
    std::cout << "Use: wiregen --language name --input_file spec.json [options]" << std::endl
              << "     wiregen --language name --input_str json_spec [options]" << std::endl
              << std::endl
              << "  --input_file filename     message specification to read" << std::endl
              << "  --input_str json          message specification given on the command line" << std::endl
              << "  --language name           language to generate: " << language_names() << std::endl
              << "                            (uses the installed lang file)" << std::endl
              << "  --lang_file filename      lang file to use instead of the installed one" << std::endl
              << "                            (--langfile is the old spelling)" << std::endl
              << "  --langspec json           lang file contents to use instead of a file" << std::endl
              << "  --namespace name          namespace (package) to generate into, replacing the spec's" << std::endl
              << "  --generate_tests          also generate unit tests" << std::endl
              << "  --generate_relay          also generate a relay program (cross-language test)" << std::endl
              << "  --generate_json           also generate to_json/from_json (C++ only for now)" << std::endl
              << "  --generate_xml            also generate to_xml/from_xml (C++ only for now)" << std::endl
              << "  --generate_bson           also generate to_bson/from_bson (C++ only for now)" << std::endl
              << "  --write_to_file           write the generated files (otherwise print them)" << std::endl
              << "  --outdir directoryname    directory to write the files to (default: .)" << std::endl
              << "  --silent                  do not print the success or failure summary" << std::endl
              << "  --banner                  print a banner with the version and revision first" << std::endl
              << "                            (on its own, print it and exit)" << std::endl
              << "  --version                 print the version and exit" << std::endl
              << "  --help                    print this help and exit" << std::endl;
}

void cli_callback(const std::string& flag, const std::string& value, void* param)
{
    if (flag.compare("--help") == 0)
    {
        print_help();
        exit(0);
    }

    if (flag.compare("--version") == 0)
    {
        std::cout << "wiregen " << skyloupe::version() << std::endl;
        exit(0);
    }

    if (flag.compare("--banner") == 0)
    {
        show_banner = true;
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

    if (flag.compare("--generate_json") == 0)
    {
        if (false != value.empty())
        {
            generate_json = true;
        }
    }

    if (flag.compare("--generate_xml") == 0)
    {
        if (false != value.empty())
        {
            generate_xml = true;
        }
    }

    if (flag.compare("--generate_bson") == 0)
    {
        if (false != value.empty())
        {
            generate_bson = true;
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

const char* const value_options[] = {
    "--input_file", "--input_str", "--outdir", "--language", "--lang_file", "--langfile", "--langspec", "--namespace",
};

const char* const flag_options[] = {
    "--help", "--version", "--banner", "--generate_tests", "--generate_relay", "--generate_json", "--generate_xml", "--generate_bson", "--silent", "--write_to_file",
};

bool process_cmd_line(int argc, const char* argv[])
{
    std::vector<std::string> cli_errors_;
    std::set<std::string> flags(std::begin(flag_options), std::end(flag_options));
    std::set<std::string> values(std::begin(value_options), std::end(value_options));

    // Callbacks run in this order, so --help and --version come first.
    for (const char* option : flag_options)
    {
        cmdargs.setOption(option, true, true, cli_callback);
    }

    for (const char* option : value_options)
    {
        cmdargs.setOption(option, true, false, cli_callback);
    }

    // The parser accepts any flag and does not say which one lacks a value,
    // so check the arguments here first; a removed or misspelled option must
    // not be ignored.
    bool ret = true;
    for (int i = 1; i < argc; i++)
    {
        const std::string arg(argv[i]);
        const bool next_is_value = i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0;

        if (values.count(arg) != 0)
        {
            if (false == next_is_value)
            {
                std::cout << arg << " needs a value (see wiregen --help)" << std::endl;
                ret = false;
            }
            else
            {
                i++;
            }
        }
        else if (flags.count(arg) != 0)
        {
            if (next_is_value)
            {
                std::cout << arg << " takes no value, but was given " << argv[i + 1] << std::endl;
                ret = false;
                i++;
            }
        }
        else if (arg.rfind("--", 0) == 0)
        {
            std::cout << "Unknown option: " << arg << " (see wiregen --help)" << std::endl;
            ret = false;
            if (next_is_value)
            {
                i++;   // most likely its value
            }
        }
        else
        {
            std::cout << "Unexpected argument: " << arg << " (see wiregen --help)" << std::endl;
            ret = false;
        }
    }

    if (ret)
    {
        ret = cmdargs.parse(argc, argv, cli_errors_);
        for (const auto& error : cli_errors_)
        {
            std::cout << error << std::endl;
        }
    }

    if (false == ret)
    {
        return ret;
    }

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

// A build keeps the lang files next to wiregen; an install puts them in
// <prefix>/share/skywire/lang, WIREGEN_LANG_RELDIR from wiregen.
std::filesystem::path installed_lang_path(const std::filesystem::path& exe_dir, const std::string& name)
{
    std::error_code ec;
    const std::filesystem::path beside = exe_dir / name;
    if (std::filesystem::exists(beside, ec))
    {
        return beside;
    }
    const std::filesystem::path shared = exe_dir / WIREGEN_LANG_RELDIR / name;
    if (std::filesystem::exists(shared, ec))
    {
        return shared;
    }
    return beside;
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

    if (argc < 2)
    {
        print_help();
        return 1;
    }

    success = process_cmd_line(argc, argv);
    if (false == success)
    {
        std::cout << "Invalid input parameters" << std::endl;
        return 1;
    }

    if (show_banner)
    {
        print_banner();
        if (input_filename.empty() && input_str.empty())
        {
            return 0;
        }
    }

    if (input_filename.empty() && input_str.empty())
    {
        std::cout << "Give a specification with --input_file or --input_str" << std::endl;
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
            lang_file = installed_lang_path(executable_dir(argv[0]), installed_lang_file).string();
        }
    }

    if (lang_file.empty() && langspec.empty())
    {
        std::cout << "Choose a language with --language (" << language_names() << ") or give a --lang_file or --langspec" << std::endl;
        return 1;
    }

    if (true == file_mode && false == langspec.empty())
    {
        success = process_file_langstr(input_filename, output_str_map, output_directory, langspec, lang_name, errorcode, generate_tests, write_to_file, generate_relay, name_space, generate_json, generate_xml, generate_bson);
    }
    else if (true == file_mode)
    {
        success = process_file(input_filename, output_str_map, output_directory, lang_file, lang_name, errorcode, generate_tests, write_to_file, generate_relay, name_space, generate_json, generate_xml, generate_bson);
    } 
    else
    {
        if (langspec.empty() && false == read_text_file(lang_file, langspec))
        {
            std::cout << "Cannot read lang file: " << lang_file << std::endl;
            return SW_FILEREADERROR;
        }

        success = process_string(input_str, output_str_map, langspec, lang_name, output_directory, errorcode, generate_tests, write_to_file, generate_relay, name_space, generate_json, generate_xml, generate_bson);
    }

    if (false == write_to_file)
    {
        for (auto item : output_str_map)
        {
            std::cout << item.second << std::endl;
        }
    }

    // String mode has no file name; the spec's JSON could be long, so name the option.
    const std::string input_name = file_mode ? input_filename : std::string("--input_str");
    if (success == true)
    {
        if(!silent) 
            std::cout << "Successful process: " << input_name << std::endl;
    }
    else
    {
        if (!silent)
        {
            std::cout << "Failure processing: " << input_name << std::endl;
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

//bin/wiregen --language c++ --generate_tests --input_str "{\"stringValues\":{\"classname\":\"Classy\",\"namespace\":\"testy\"},\"members\":{\"100\":{\"type\":\"float\",\"name\":\"a\"},\"200\":{\"type\":\"uint32_t\",\"name\":\"b\"}}}"
