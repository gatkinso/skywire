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

// Reads a Telemetry from XML written by another program (telemetry.xml: a
// declaration, comments, indentation, CDATA, an entity, a string in
// hexadecimal and an element the message does not have), writes it back in
// the canonical form, and shows what from_xml refuses. See README.md.
//
//   example_xml_cpp telemetry.xml out.xml

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

    // The root element must be the message's name; members may come in any
    // order, and an element the message does not have (firmware) is ignored.
    telemetry::Telemetry msg;
    if (!msg.from_xml(text))
    {
        std::fprintf(stderr, "%s is not the XML form of a Telemetry\n", argv[1]);
        return 1;
    }
    const unsigned char* callsign = reinterpret_cast<const unsigned char*>(msg.Get_callsign());
    std::printf("device_id %u, callsign bytes %02x %02x %02x %02x %02x, altitude_m %.1f\n", msg.Get_device_id(),
                callsign[0], callsign[1], callsign[2], callsign[3], callsign[4],
                double(msg.Get_position().Get_altitude_m()));

    // to_xml writes the canonical form: no declaration, no whitespace, and
    // the callsign in hexadecimal again, as it holds a byte that is not
    // printable ASCII.
    const std::string canonical = msg.to_xml();
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
        R"(<!DOCTYPE Telemetry [<!ENTITY x "y">]><Telemetry/>)",            // no document type declarations
        R"(<Position/>)",                                                   // not a Telemetry
        R"(<Telemetry><mode>5</Telemetry>)",                                // not well-formed
        R"(<Telemetry><status>8</status></Telemetry>)",                     // status has 3 bits
        R"(<Telemetry><callsign encoding="base64">U0tZMQ==</callsign></Telemetry>)", // hex only
    };
    for (const char* bad : refused)
    {
        const telemetry::Telemetry before = msg;
        if (msg.from_xml(bad) || !(msg == before))
        {
            std::fprintf(stderr, "from_xml accepted %s\n", bad);
            return 1;
        }
        std::printf("refused %s\n", bad);
    }
    return 0;
}
