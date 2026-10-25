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

// Fuzzes wiregen's message specification handling: the input is a spec's
// JSON, which is parsed, validated and generated (code and unit tests) for
// every language, as `wiregen --generate_tests` does, without writing files.
//
// Properties, for any input: no crash, no sanitizer report, no exception
// from a generator, and a spec that is accepted produces output.

#include <iostream>
#include <string>
#include <vector>

#include "fuzz_wiregen.h"
#include "processor.h"

namespace
{

std::vector<std::string> lang_files;

}

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    // Generators report a bad spec on std::cerr; keep the fuzzer's log readable.
    std::cerr.rdbuf(nullptr);
    lang_files = fuzz::read_lang_files();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string spec(reinterpret_cast<const char*>(data), size);

    for (const auto& lang : lang_files)
    {
        skyloupe::Processor processor;
        skyloupe::results_t results;
        int errorcode = 0;
        if (processor.process_string(spec, lang, results, "", errorcode, true))
        {
            FUZZ_CHECK(false == results.empty());
        }
        // The shipped lang files are good, so a generator that throws on a
        // spec (reported as an invalid lang file) is a generator bug.
        FUZZ_CHECK((errorcode & SW_LANG_INVALID) == 0);
    }
    return 0;
}
