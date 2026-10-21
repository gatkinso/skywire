//
//   Copyright (C) 2019 Geoffrey Atkinson
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

// Writes a damaged copy of a serialized message, so the serial test can check
// that every relay rejects corrupt input (CMake cannot write binary files).
//
//     ring_tamper flip <in> <out> <offset>   -- invert every bit of one byte
//     ring_tamper truncate <in> <out>        -- drop the last byte
//     ring_tamper extend <in> <out>          -- append one zero byte

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::fprintf(stderr, "usage: %s flip|truncate|extend <in> <out> [offset]\n", argv[0]);
        return 2;
    }

    std::ifstream in(argv[2], std::ios::binary);
    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (!in.is_open() || data.empty())
    {
        std::fprintf(stderr, "cannot read %s\n", argv[2]);
        return 1;
    }

    if (std::strcmp(argv[1], "flip") == 0 && argc == 5)
    {
        const unsigned long offset = std::strtoul(argv[4], nullptr, 10);
        if (offset >= data.size())
        {
            std::fprintf(stderr, "offset %lu is past the end (%zu bytes)\n", offset, data.size());
            return 1;
        }
        data[offset] = static_cast<char>(~data[offset]);
    }
    else if (std::strcmp(argv[1], "truncate") == 0)
    {
        data.pop_back();
    }
    else if (std::strcmp(argv[1], "extend") == 0)
    {
        data.push_back(0);
    }
    else
    {
        std::fprintf(stderr, "unknown mode %s\n", argv[1]);
        return 2;
    }

    std::ofstream out(argv[3], std::ios::binary);
    out.write(data.data(), data.size());
    out.close();
    return out ? 0 : 1;
}
