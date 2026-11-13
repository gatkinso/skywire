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

// Reads a Telemetry from JSON written by hand (telemetry.json: indented,
// members in another order, one missing and one the message does not have),
// writes it back in the canonical form, and shows what from_json refuses.
// See README.md.
//
//   example_json_cpp telemetry.json out.json

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "Telemetry.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::fprintf(stderr, "usage: %s INPUT OUTPUT\n", argv[0]);
        return 2;
    }

    std::ifstream in(argv[1], std::ios::binary);
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    // from_json takes members in any order, leaves a missing one (battery_mv)
    // zero and ignores one the message does not have (firmware). A 64-bit
    // integer may be a JSON string, as to_json writes it.
    telemetry::Telemetry msg;
    if (!msg.from_json(text))
    {
        std::fprintf(stderr, "%s is not the JSON form of a Telemetry\n", argv[1]);
        return 1;
    }
    std::printf("device_id %u, callsign %.8s, timestamp_us %llu, battery_mv %u\n", msg.Get_device_id(),
                msg.Get_callsign(), static_cast<unsigned long long>(msg.Get_timestamp_us()),
                unsigned(msg.Get_battery_mv()));

    // to_json writes the canonical form: no whitespace, members in tag order.
    const std::string canonical = msg.to_json();
    std::ofstream out(argv[2], std::ios::binary);
    if (!out.write(canonical.data(), canonical.size()))
    {
        std::fprintf(stderr, "cannot write %s\n", argv[2]);
        return 1;
    }
    std::printf("%s\n", canonical.c_str());

    // Input that is not a Telemetry, or holds a value a member cannot, is
    // refused, and the message is left as it was.
    const char* const refused[] = {
        R"({"status": 9})",                       // status has 3 bits
        R"({"device_id": -1})",                   // a uint32_t
        R"({"timestamp_us": 1.5})",               // not a whole number
        R"({"callsign": "TOO-LONG-CALLSIGN"})",   // more than 8 bytes
        R"({"armed": true})",                     // not a number
        R"({"mode": 5,})",                        // not JSON
    };
    for (const char* bad : refused)
    {
        const telemetry::Telemetry before = msg;
        if (msg.from_json(bad) || !(msg == before))
        {
            std::fprintf(stderr, "from_json accepted %s\n", bad);
            return 1;
        }
        std::printf("refused %s\n", bad);
    }
    return 0;
}
