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
#include "generators.h"

static std::string language_key("language");
static std::string cpp_str("C++");

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

bool parse_file(const std::string pathname, int& error)
{
    skyloupe::Stencil stencil;

    if (false == stencil.read_file(pathname))
    {
        error = 1;
        std::cerr << "Error reading file: " << pathname << std::endl;
        return false;
    }

    std::string lang = stencil.get_value_string(language_key);

    // temporary - replace with factory, dynamic language definition via json
    std::unique_ptr<skyloupe::Generator> genny;
    if (lang == cpp_str)
    {
        genny = std::make_unique<skyloupe::CppGenerator>(stencil);
    }
    else
    {
        error = 2;
        std::cerr << "Unsupported language: " << lang << std::endl;
        return false;
    }
    // end temporary 



    return true;
}