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

#include "skywire.h"
#include "processor.h"

namespace skyloupe
{

bool process_file(const std::string& pathname, const std::string& outdirname, const std::string& langfile, int& errorcode, bool generate_tests)
{
    skyloupe::Processor processor;

    if(!langfile.empty())
        processor.set_langfile(langfile);

    bool success = processor.process_file(pathname, outdirname, errorcode, generate_tests);

    return success;
}

bool process_string(const std::string& input_str, results_t& output_str_map, const std::string& langfile, int& errorcode, bool generate_tests)
{
    skyloupe::Processor processor;

    if(!langfile.empty())
        processor.set_langfile(langfile);
            
    bool success = processor.process_string(input_str, output_str_map, errorcode, generate_tests);

    return success;
}

}