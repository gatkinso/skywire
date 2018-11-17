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

bool process_file(const std::string& pathname, const std::string& outdirname, int& error, const std::string lang_pathname)
{
    skyloupe::Processor processor;

    bool success = processor.process_file(pathname, outdirname, error);

    return success;
}

}