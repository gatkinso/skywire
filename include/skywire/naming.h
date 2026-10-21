//
//   Copyright (C) 2026 Geoffrey Atkinson
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

namespace skyloupe
{

// An identifier in snake_case: lower case, with an underscore where a lower
// case letter or digit is followed by an upper case one, or before the last
// capital of an acronym ("RingyMcRingface" -> "ringy_mc_ringface",
// "HTTPHeader" -> "http_header", "CNestyInner" -> "c_nesty_inner"). Existing
// underscores are kept. This is the {classname_snake} file name token and
// the name of a generated Rust module.
std::string snake_case(const std::string& name);

}
