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

namespace skyloupe 
{

class Generator
{
public:
    Generator(Stencil& stencil) : msg_stencil_(stencil) {}
    virtual ~Generator() {}

    virtual bool generate(int& error) = 0;
    virtual bool set_lang_settings(const std::string pathname);
    virtual bool set_msg_settings(const std::string pathname);

protected:
    Stencil lang_stencil_;
    Stencil msg_stencil_;
    std::string result_;
    int error_;
};

}
