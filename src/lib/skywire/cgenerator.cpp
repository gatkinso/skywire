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

#include "cgenerator.h"

namespace skyloupe
{

bool CGenerator::generate(int& errorcode)
{
    std::cout << "Generating C" << std::endl;
    errorcode = 0;

    if (false == do_header(errorcode)) return false;
    if (false == do_implementation(errorcode)) return false;
    return true;
}

bool CGenerator::do_header(int& errorcode)
{
    return true;
}

bool CGenerator::do_implementation(int& errorcode)
{
    return true;
}

}