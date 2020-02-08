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
#include "specification/specification.h"

#include "TransportClient.h"
#include "stencil.h"

using namespace skyloupe;

qwikpirate::ArgVee cmdargs;
std::string config_pathname = "settings.json";
std::string input_filename;
std::string input_str;
std::string output_directory = ".";
results_t output_str_map;
std::string lang_file;
std::string lang_str;
bool silent = false;
bool generate_tests = false;
bool file_mode = false;
bool has_lang_file = false;
bool has_lang_spec = false;

void cli_callback(const std::string& flag, const std::string& value, void* param)
{
    if (flag.compare("--help") == 0)
    {
        std::cout << "Use: wirecloud  (no paramters)               -- defaults to using settings.json" << std::endl
                  << "     wirecloud --version                     -- prints version and exits" << std::endl
                  << "     wirecloud --config filename             -- launches using fiename for configuration file" << std::endl
                  << "     wirecloud --input_file filename         -- launches using filename for input" << std::endl
                  << "     wirecloud --input_str json_msg_spec     -- launches using provided json string for input" << std::endl
                  << "     wirecloud --outdir directoryname        -- directory to write generated file(s) to" << std::endl
                  << "     wirecloud --lang_file language filename -- language file to be used" << std::endl
                  << "     wirecloud --lang_str json_lang_spec     -- language file to be used" << std::endl
                  << "     wirecloud --generate_tests              -- generate unit tests" << std::endl
                  << "     wirecloud --silent                      -- no output displayed" << std::endl;
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

    if (flag.compare("--lang_file") == 0)
    {
        if (false == value.empty())
        {
            lang_file = value;
            has_lang_file = true;
        }
    }

    if (flag.compare("--lang_str") == 0)
    {
        if( has_lang_file == true )
        {
            std::cout << "Can only specify language file or string flags, not both" << std::endl;
            exit(1);
        }

        if (false == value.empty())
        {
            lang_file = value;
            has_lang_spec = true;
        }
    }

    if (flag.compare("--silent") == 0)
    {
        if (false == value.empty())
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
    cmdargs.setOption("--lang_file", true, false, cli_callback);
    cmdargs.setOption("--lang_str", true, false, cli_callback);
    cmdargs.setOption("--generate_tests", true, true, cli_callback);
    cmdargs.setOption("--silent", true, true, cli_callback);

    bool ret = cmdargs.parse(argc, argv, cli_errors_);

    cmdargs.apply();

    if( has_lang_file == false && has_lang_spec == false )
    {
        std::cout << "Need --lang_file or --lang_str (never both!)" << std::endl;
        exit(1);
    }

    return ret;
}

int main(int argc, const char* argv[])
{
    bool success = false;
    int errorcode = 0;

    success = process_cmd_line(argc, argv);
    if (false == success)
    {
        std::cout << "Invalid input parameters" << std::endl;
        return success;
    }

    Specification msg_spec, lang_spec;

    if (true == file_mode)
    {
        if (false == msg_spec.read_file(input_filename))
        {
            errorcode |= SW_FILEREADERROR;
            std::cerr << "Error reading file: " << input_filename << std::endl;
            return success;
        }
    }
    else
    {
        if (false == msg_spec.read_string(input_str))
        {
            errorcode |= SW_STRINGREADERROR;
            std::cerr << "Error reading string: " << input_str << std::endl;
            return success;
        }
    }

    if( has_lang_file == true )
    {
        if (false == lang_spec.read_file(lang_file))
        {
            errorcode |= SW_FILEREADERROR;
            std::cerr << "Error reading file: " << input_filename << std::endl;
            return success;
        }
    } else {
        if (false == lang_spec.read_string(lang_str))
        {
            errorcode |= SW_STRINGREADERROR;
            std::cerr << "Error reading file: " << input_filename << std::endl;
            return success;
        }
    }

    TransportClient client(
        grpc::CreateChannel("localhost:1967", grpc::InsecureChannelCredentials())
    );

    Request req;
    req.add_impl()->PackFrom(msg_spec.get_proto());

    Response res;
    auto status = client.Exchange(req, res);

    std::cout << "Response received: " << res.DebugString() << std::endl;

    return errorcode;
}

//bin/wirecloud --lang_file ../lang/lang.cpp.json --generate_tests --input_str "{\"stringValues\":{\"classname\":\"Classy\",\"outputheaderfilename\":\"Classy.h\",\"outputtestfilename\":\"ClassyTest.cpp\",\"namespace\":\"testy\",\"language\":\"c++\",\"lang_file\":\"lang.cpp.json\"},\"members\":{\"100\":{\"type\":\"float\",\"name\":\"a\"},\"200\":{\"type\":\"uint32_t\",\"name\":\"b\"}},\"includes\":[\"<streamable.h>\",\"<stdint.h>\"]}"
