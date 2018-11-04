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
#include <memory>
#include "stencil/stencil.h"
#include "skywire.h"
#include "processor.h"

namespace skyloupe
{

bool skywire_init()
{
    std::cout << "Skywire initializing!" << std::endl;

    return true;
}

bool skywire_fini()
{
    std::cout << "Skywire shutting down!" << std::endl;

    return true;
}

bool process_file(const std::string pathname, int& error)
{
    skyloupe::Stencil stencil;
    skyloupe::Processor processor(stencil);

    bool success = processor.process_file(pathname, error);

    return success;
}

}