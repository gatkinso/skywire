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

// Fuzzes wiregen's lang file handling: the input is a lang file's JSON, and
// fixed message specs are generated (code and unit tests) with it. The ring
// spec has every native type, strings, a nested message and bitfields; the
// lang file chooses the language, output file names, includes and the
// native type list.
//
// Properties, for any input: no crash, no sanitizer report, and a lang file
// that is accepted produces output.

#include <iostream>
#include <string>
#include <vector>

#include "fuzz_wiregen.h"
#include "processor.h"

namespace
{

std::vector<std::string> specs;

}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    // Generators report a bad lang file on std::cerr; keep the fuzzer's log readable.
    std::cerr.rdbuf(nullptr);
    specs.push_back(fuzz::read_file(SKYWIRE_RING_SPEC));
    specs.push_back(fuzz::read_file(SKYWIRE_RING_INNER_SPEC));
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string lang(reinterpret_cast<const char*>(data), size);

    for (const auto& spec : specs)
    {
        skyloupe::Processor processor;
        skyloupe::results_t results;
        int errorcode = 0;
        if (processor.process_string(spec, lang, results, "", errorcode, true))
        {
            FUZZ_CHECK(false == results.empty());
        }
    }
    return 0;
}
