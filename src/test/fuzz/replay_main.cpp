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

// The main() a fuzz target gets when it is built without libFuzzer: it runs
// LLVMFuzzerTestOneInput once on every file named on the command line, or in
// a directory named there, so ctest replays the corpora on every platform.
// A target reports a failed property by aborting, as it does under libFuzzer.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);

namespace fs = std::filesystem;

static bool run_one(const fs::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        std::cerr << "cannot read " << path.string() << std::endl;
        return false;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::cout << "replaying " << path.string() << " (" << bytes.size() << " bytes)" << std::endl;
    LLVMFuzzerTestOneInput(bytes.data(), bytes.size());
    return true;
}

int main(int argc, char** argv)
{
    LLVMFuzzerInitialize(&argc, &argv);

    size_t inputs = 0;
    for (int i = 1; i < argc; ++i)
    {
        const fs::path arg(argv[i]);
        std::vector<fs::path> files;
        if (fs::is_directory(arg))
        {
            for (const auto& entry : fs::directory_iterator(arg))
            {
                if (entry.is_regular_file() && entry.path().filename().string()[0] != '.')
                {
                    files.push_back(entry.path());
                }
            }
            std::sort(files.begin(), files.end());
        }
        else
        {
            files.push_back(arg);
        }

        for (const auto& file : files)
        {
            if (!run_one(file))
            {
                return 1;
            }
            ++inputs;
        }
    }

    if (inputs == 0)
    {
        std::cerr << "no inputs" << std::endl;
        return 1;
    }
    std::cout << inputs << " inputs passed" << std::endl;
    return 0;
}
