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

// The reference for skywire_text_diff: reads inputs (each a 4-byte
// little-endian length and the bytes) from the file named, and prints, for
// each, three lines: what the generated C++ ring message makes of it as
// JSON, as XML and as BSON. "J -" if from_json refuses it, otherwise "J "
// and the hex of what to_json writes back; likewise "X" and "B". A driver
// in another language prints the same for the same inputs.

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "cpp_text/RingyMcRingface.h"

namespace
{

std::string hex(const std::string& s)
{
    static const char digits[] = "0123456789abcdef";
    std::string out;
    for (unsigned char c : s)
    {
        out += digits[c >> 4];
        out += digits[c & 15];
    }
    return out;
}

}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: text_diff_oracle <inputs>" << std::endl;
        return 2;
    }
    std::ifstream in(argv[1], std::ios::binary);
    std::ios::sync_with_stdio(false);
    unsigned char prefix[4];
    while (in.read(reinterpret_cast<char*>(prefix), 4))
    {
        const uint32_t n = prefix[0] | (prefix[1] << 8) | (prefix[2] << 16) | (uint32_t(prefix[3]) << 24);
        std::string d(n, '\0');
        in.read(&d[0], n);
        ring::RingyMcRingface json, xml, bson;
        std::cout << (json.from_json(d) ? "J " + hex(json.to_json()) : "J -") << "\n";
        std::cout << (xml.from_xml(d) ? "X " + hex(xml.to_xml()) : "X -") << "\n";
        std::cout << (bson.from_bson(d) ? "B " + hex(bson.to_bson()) : "B -") << "\n";
    }
    return 0;
}
