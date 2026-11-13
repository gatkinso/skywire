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

// The text of a runtime file wiregen writes beside the generated code, by
// its path under lang/ ("python/skywire_text.py"); empty if there is none.
// Built into wiregen from lang/ (embed_runtime.cmake).
const std::string& runtime_source(const std::string& name);

}
