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

// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//   example_cpp telemetry.bin

#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

#include "stream.h"
#include "Telemetry.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::fprintf(stderr, "usage: %s FILE\n", argv[0]);
        return 2;
    }

    telemetry::Position position;
    position.Set_latitude(47.6062);
    position.Set_longitude(-122.3321);
    position.Set_altitude_m(56.5f);

    telemetry::Telemetry sent;
    sent.Set_device_id(4207);
    sent.Set_timestamp_us(1760959800123456ULL);
    sent.Set_mode(5);
    sent.Set_status(11); // status has 3 bits: 11 is stored as 3
    sent.Set_armed(1);
    sent.Set_callsign("SKY1"); // NUL-padded to 8 bytes
    sent.Set_position(position);
    sent.Set_battery_mv(3870);

    // Serialize into a stream, then write the stream's bytes to the file.
    skyloupe::Stream stream;
    if (!sent.serialize(stream))
    {
        std::fprintf(stderr, "serialize failed\n");
        return 1;
    }
    std::vector<char> bytes(stream.byte_size());
    stream.read(bytes.data(), bytes.size());
    {
        std::ofstream out(argv[1], std::ios::binary);
        if (!out.write(bytes.data(), bytes.size()))
        {
            std::fprintf(stderr, "cannot write %s\n", argv[1]);
            return 1;
        }
    }

    // Read the file back into the stream and deserialize a second message.
    std::ifstream in(argv[1], std::ios::binary);
    std::vector<char> back((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    stream.write(back.data(), back.size());
    telemetry::Telemetry received;
    if (!received.deserialize(stream))
    {
        std::fprintf(stderr, "deserialize failed\n");
        return 1;
    }

    if (!(received == sent))
    {
        std::fprintf(stderr, "the message read back is different\n");
        return 1;
    }

    std::printf("wrote and read back %zu bytes (size() = %zu)\n", back.size(), received.size());
    std::printf("device_id %u, callsign %.8s, status %u, position %.4f, %.4f, %.1f m\n",
                received.Get_device_id(), received.Get_callsign(), unsigned(received.Get_status()),
                received.Get_position().Get_latitude(), received.Get_position().Get_longitude(),
                double(received.Get_position().Get_altitude_m()));
    return 0;
}
