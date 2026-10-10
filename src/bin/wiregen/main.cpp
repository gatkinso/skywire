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

#include <iostream>
#include <string>
#include "argv.h"
#include "skywire/skywire.h"

using namespace skyloupe;

qwikpirate::ArgVee cmdargs;
std::string config_pathname = "settings.json";
std::string input_filename;
std::string input_str;
std::string output_directory = ".";
results_t output_str_map;
std::string langfile;
std::string langspec;
bool silent = false;
bool generate_tests = false;
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
                  << "     wiregen --langfile filename           -- language filename (if --input_file is specified) to be used" << std::endl
                  << "     wiregen --langspec string             -- language string (if --input_str is specified) to be used" << std::endl
                  << "     wiregen --generate_tests              -- generate unit tests" << std::endl
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

    if (flag.compare("--langfile") == 0)
    {
        if (false == value.empty())
        {
            langfile = value;
        }
    }

    if (flag.compare("--langspec") == 0)
    {
        if (false == value.empty())
        {
            langspec = value;
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
    cmdargs.setOption("--langfile", true, false, cli_callback);
    cmdargs.setOption("--langspec", true, false, cli_callback);
    cmdargs.setOption("--generate_tests", true, true, cli_callback);
    cmdargs.setOption("--silent", true, true, cli_callback);
    cmdargs.setOption("--write_to_file", true, true, cli_callback);

    bool ret = cmdargs.parse(argc, argv, cli_errors_);

    cmdargs.apply();

    return ret;
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

    if (true == file_mode)
    {
        success = process_file(input_filename, output_directory, langfile, errorcode, generate_tests, write_to_file);
    } 
    else
    {
        success = process_string(input_str, output_str_map, langspec, output_directory, errorcode, generate_tests, write_to_file);

        if (false == write_to_file) {
            for (auto item : output_str_map)
            {
                std::cout << item.second << std::endl;
            }
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

//bin/wiregen --langfile ../lang/lang.cpp.json --generate_tests --input_str "{\"stringValues\":{\"classname\":\"Classy\",\"outputheaderfilename\":\"Classy.h\",\"outputtestfilename\":\"ClassyTest.cpp\",\"namespace\":\"testy\",\"language\":\"c++\",\"lang_file\":\"lang.cpp.json\"},\"members\":{\"100\":{\"type\":\"float\",\"name\":\"a\"},\"200\":{\"type\":\"uint32_t\",\"name\":\"b\"}},\"includes\":[\"<streamable.h>\",\"<stdint.h>\"]}"
