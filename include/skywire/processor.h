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

#pragma once

#include <string>
#include "stencil/stencil.h"
#include "generators.h"
#include "errorcodes.h"

namespace skyloupe 
{

class Processor
{
public:
    Processor() {}
    virtual ~Processor() {}

    virtual bool process_file(const std::string& pathname, const std::string& outputdir, int& errorcode);

private:
    Stencil msg_stencil_;

    std::string language_key = "language";

    std::string ada_str = "ada";
    std::string c_str = "c";
    std::string cpp_str = "c++";
    std::string java_str = "java";
    std::string py_str = "python";
};

}
