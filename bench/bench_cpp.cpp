//
//   Copyright (C) 2026 Skyloupe, LLC
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

// Times the generated C++ code (SW mode) for the ring message. See README.md.
//
//   bench_cpp RingyMcRingface.bin

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

#include "stream.h"
#include "RingyMcRingface.h"

namespace {

constexpr int kRounds = 5;
constexpr long kIters = 2000000;

volatile long sink;

// Best time per iteration, in nanoseconds, over kRounds rounds of body().
template <typename F>
double best_ns(F body)
{
    double best = 1e30;
    for (int r = 0; r < kRounds; r++)
    {
        auto t = std::chrono::steady_clock::now();
        for (long i = 0; i < kIters; i++)
        {
            body();
        }
        std::chrono::duration<double, std::nano> d = std::chrono::steady_clock::now() - t;
        best = std::min(best, d.count() / kIters);
    }
    return best;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::fprintf(stderr, "usage: %s RingyMcRingface.bin\n", argv[0]);
        return 2;
    }
    std::ifstream f(argv[1], std::ios::binary);
    if (!f)
    {
        std::fprintf(stderr, "cannot open %s\n", argv[1]);
        return 2;
    }
    std::vector<char> bin((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    skyloupe::Stream s(4096, 4096);
    ring::RingyMcRingface m, m2;

    // The message must decode and re-encode to the same bytes.
    s.write(bin.data(), bin.size());
    if (!m.deserialize(s) || s.byte_size() != 0)
    {
        std::fprintf(stderr, "deserialize failed\n");
        return 1;
    }
    m.serialize(s);
    std::vector<char> out(s.byte_size());
    s.read(out.data(), out.size());
    if (out != bin)
    {
        std::fprintf(stderr, "serialize does not reproduce the input\n");
        return 1;
    }

    double ser = best_ns([&] {
        m.serialize(s);
        sink += s.byte_size();
        s.clear();
    });
    double de = best_ns([&] {
        s.write(bin.data(), bin.size());
        m2.deserialize(s);
        sink += m2.Get_u8();
    });
    double reuse = best_ns([&] {
        m.serialize(s);
        m2.deserialize(s);
        sink += m2.Get_u8();
    });

    std::printf("RESULT serialize %.1f\n", ser);
    std::printf("RESULT deserialize %.1f\n", de);
    std::printf("NOTE reused-stream round trip %.1f ns\n", reuse);
    return 0;
}
