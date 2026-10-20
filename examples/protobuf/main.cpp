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

// Sends a skywire Telemetry message to protobuf and back, through a PZ-mode
// stream. The other side is protoc's C++ code for the .proto files wiregen
// generates (--language proto3), in namespace telemetry::pb. See README.md.
//
//   example_protobuf telemetry.pb

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>

#include "stream.h"
#include "Telemetry.h"     // skywire: telemetry::Telemetry
#include "Telemetry.pb.h"  // protoc:  telemetry::pb::Telemetry

namespace {

int fail(const char* message)
{
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

} // namespace

int main(int argc, char** argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

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
    sent.Set_callsign("SKY1"); // goes to protobuf as "SKY1", without the NUL padding
    sent.Set_position(position);
    sent.Set_battery_mv(3870);

    // 1. skywire -> protobuf. A PZ-mode stream writes each member as the
    //    protobuf field numbered by its tag, into pz_buffer().
    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    if (!sent.serialize(stream))
    {
        return fail("PZ serialize failed");
    }
    const std::string wire = stream.pz_buffer();
    stream.clear();
    {
        std::ofstream out(argv[1], std::ios::binary);
        if (!out.write(wire.data(), wire.size()))
        {
            return fail("cannot write the file");
        }
    }

    std::ifstream in(argv[1], std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    telemetry::pb::Telemetry pb;
    if (!pb.ParseFromString(bytes))
    {
        return fail("protobuf cannot parse what skywire wrote");
    }
    if (pb.device_id() != 4207 || pb.status() != 3 || pb.callsign() != "SKY1" ||
        pb.position().altitude_m() != 56.5f || pb.battery_mv() != 3870)
    {
        return fail("protobuf read different values");
    }
    // The PZ encoding is canonical proto3, the same bytes protobuf writes.
    if (pb.SerializeAsString() != bytes)
    {
        return fail("protobuf re-encodes the message differently");
    }
    std::printf("protobuf read %zu bytes: device_id %u, callsign %s, status %u, altitude %.1f m\n",
                bytes.size(), pb.device_id(), pb.callsign().c_str(), pb.status(),
                double(pb.position().altitude_m()));

    // 2. protobuf -> skywire. Change the message on the protobuf side, then
    //    decode it with the same generated skywire class. Put the bytes in
    //    pz_buffer(): deserialize decodes all of it as one message.
    pb.set_battery_mv(3700);
    pb.mutable_position()->set_altitude_m(61.0f);
    stream.pz_buffer() = pb.SerializeAsString();
    telemetry::Telemetry received;
    if (!received.deserialize(stream))
    {
        return fail("PZ deserialize failed");
    }

    telemetry::Telemetry expected = sent;
    expected.Set_battery_mv(3700);
    position.Set_altitude_m(61.0f);
    expected.Set_position(position);
    if (!(received == expected))
    {
        return fail("skywire read different values");
    }
    std::printf("skywire read it back: battery_mv %u, altitude %.1f m\n",
                unsigned(received.Get_battery_mv()), double(received.Get_position().Get_altitude_m()));

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
