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
std::string message_pathname;
std::string output_directory = ".";
std::string langfile_directory = ".";

void cli_callback(const std::string& flag, const std::string& value, void* param)
{
    if (flag.compare("--help") == 0)
    {
        std::cout << "Use: wiregen  (no paramters)               -- defaults to using settings.json" << std::endl
            << "      wiregen --version                    -- prints version and exits" << std::endl
            << "      wiregen --config filename            -- launches using fiename for configuration file" << std::endl
            << "      wiregen --input filename             -- launches using filename for input" << std::endl
            << "      wiregen --outdir directoryname       -- directory to write generated file(s) to" << std::endl
            << "      wiregen --langfiledir directoryname  -- directory containing language file" << std::endl;
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

    if (flag.compare("--input") == 0)
    {
        if (false == value.empty())
        {
            message_pathname = value;
        }
        else
        {
            std::cout << "Need a file name for --input switch" << std::endl;
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

    if (flag.compare("--langfiledir") == 0)
    {
        if (false == value.empty())
        {
            langfile_directory = value;
        }
        else
        {
            std::cout << "Need a valid language directory name" << std::endl;
        }
    }
}

bool process_cmd_line(int argc, const char* argv[])
{
    std::vector<std::string> cli_errors_;

    cmdargs.setOption("--help", true, true, cli_callback);
    cmdargs.setOption("--version", true, true, cli_callback);
    cmdargs.setOption("--config", true, false, cli_callback);
    cmdargs.setOption("--input", true, false, cli_callback);
    cmdargs.setOption("--outdir", true, false, cli_callback);
    cmdargs.setOption("--langfiledir", true, false, cli_callback);

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
        std::cout << "Invalid parameters" << std::endl;
        return success;
    }

    success = process_file(message_pathname, output_directory, langfile_directory, errorcode);
    if (success == true)
    {
        std::cout << "Successful process: " << message_pathname << std::endl;
    }
    else
    {
        std::cout << "Failure processing: " << message_pathname << std::endl;
        std::cout << "Error number: " << errorcode << std::endl;
    }

	return 0;
}