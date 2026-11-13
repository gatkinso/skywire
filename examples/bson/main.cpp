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

// Reads a Telemetry from a BSON document as a MongoDB driver might hand it
// back (telemetry.bson: an _id and a received_at the message does not have,
// members in another order, numbers in other types), writes it back in the
// canonical form, shows a uint64_t above 2^63-1 written as a Decimal128, and
// shows what from_bson refuses. See README.md.
//
//   example_bson_cpp telemetry.bson out.bson

#include <cstdint>
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
    const std::string doc((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    // from_bson ignores elements the message does not have (_id,
    // received_at) and takes any number type that holds the member's value:
    // here an int32 device_id and a double battery_mv.
    telemetry::Telemetry msg;
    if (!msg.from_bson(doc))
    {
        std::fprintf(stderr, "%s is not the BSON form of a Telemetry\n", argv[1]);
        return 1;
    }
    std::printf("device_id %u, callsign %.8s, battery_mv %u\n", msg.Get_device_id(), msg.Get_callsign(),
                unsigned(msg.Get_battery_mv()));

    // to_bson writes the canonical document: members in tag order, each in
    // the one type its member's type maps to (uint32_t device_id as int64,
    // uint16_t battery_mv as int32).
    const std::string canonical = msg.to_bson();
    std::ofstream out(argv[2], std::ios::binary);
    if (!out.write(canonical.data(), canonical.size()))
    {
        std::fprintf(stderr, "cannot write %s\n", argv[2]);
        return 1;
    }
    std::printf("read %zu bytes, wrote %zu\n", doc.size(), canonical.size());

    // A uint64_t above 2^63-1 is written as a Decimal128 (type 0x13), so no
    // tool reads it as a negative int64.
    telemetry::Telemetry big = msg;
    big.Set_timestamp_us(UINT64_MAX);
    const std::string big_doc = big.to_bson();
    const std::string head = std::string("\x13") + "timestamp_us" + std::string(1, '\0');
    telemetry::Telemetry back;
    if (big_doc.find(head) == std::string::npos || !back.from_bson(big_doc) || !(back == big))
    {
        std::fprintf(stderr, "timestamp_us %llu did not go through as a Decimal128\n",
                     static_cast<unsigned long long>(UINT64_MAX));
        return 1;
    }
    std::printf("timestamp_us %llu is a Decimal128, and reads back\n", static_cast<unsigned long long>(UINT64_MAX));

    // A document that is not a Telemetry, or holds a value a member cannot,
    // is refused, and the message is left as it was.
    const std::string refused[][2] = {
        { doc.substr(0, doc.size() - 1), "cut short" },
        { doc + std::string(1, '\0'), "a byte after the document" },
        { std::string("\x11\0\0\0\x10status\0\x09\0\0\0\0", 17), "{status: int32 9}: status has 3 bits" },
        { std::string("\x18\0\0\0\x12" "device_id\0\xff\xff\xff\xff\xff\xff\xff\xff\0", 24),
          "{device_id: int64 -1}: a uint32_t" },
        { std::string("\x1b\0\0\0\x01timestamp_us\0\0\0\0\0\0\0\xf8\x3f\0", 27),
          "{timestamp_us: double 1.5}: not a whole number" },
    };
    for (const auto& bad : refused)
    {
        const telemetry::Telemetry before = msg;
        if (msg.from_bson(bad[0]) || !(msg == before))
        {
            std::fprintf(stderr, "from_bson accepted a document: %s\n", bad[1].c_str());
            return 1;
        }
        std::printf("refused %s\n", bad[1].c_str());
    }
    return 0;
}
