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

// The property the JSON, XML and BSON fuzzers check, given the generated C++
// ring message with --generate_json, --generate_xml and --generate_bson. The
// input is any byte string, read as JSON, XML or BSON: malformed, deeply
// nested, bad escapes, bad UTF-8, bad lengths, out-of-range numbers, overlong
// strings, unknown and repeated members.
//
// For any input:
//   - from_json()/from_xml()/from_bson() returns false, leaving the message
//     as it was, or true;
//   - a decoded message's JSON and BSON are canonical: they read back, and
//     give the same JSON and BSON again;
//   - the other forms and the SW wire format carry it unchanged.

#pragma once

#include <cstring>
#include <string>

#include "fuzz_check.h"
#include "cpp_text/RingyMcRingface.h"

namespace fuzz_text
{

inline ring::RingyMcRingface sentinel()
{
    ring::RingyMcRingface message;
    message.Set_u8(7);
    message.Set_i64_be(-1);
    message.Set_str_exact("fuzzing");
    return message;
}

enum Format { Json, Xml, Bson };

template <Format F>
int one_input(const uint8_t* data, size_t size)
{
    const std::string input(reinterpret_cast<const char*>(data), size);

    ring::RingyMcRingface message = sentinel();
    const bool ok = F == Json ? message.from_json(input) : F == Xml ? message.from_xml(input) : message.from_bson(input);
    if (!ok)
    {
        FUZZ_CHECK(message == sentinel());
        return 0;
    }

    const std::string json = message.to_json();
    const std::string xml = message.to_xml();
    const std::string bson = message.to_bson();

    ring::RingyMcRingface from_json, from_xml;
    FUZZ_CHECK(from_json.from_json(json));
    FUZZ_CHECK(from_json.to_json() == json);
    FUZZ_CHECK(from_xml.from_xml(xml));
    FUZZ_CHECK(from_xml.to_json() == json);
    FUZZ_CHECK(from_xml.to_xml() == xml);

    ring::RingyMcRingface from_bson;
    FUZZ_CHECK(from_bson.from_bson(bson));
    FUZZ_CHECK(from_bson.to_bson() == bson);
    FUZZ_CHECK(from_bson.to_json() == json);

    skyloupe::Stream sw;
    FUZZ_CHECK(message.serialize(sw));
    ring::RingyMcRingface from_sw;
    FUZZ_CHECK(from_sw.deserialize(sw));
    FUZZ_CHECK(from_sw.to_json() == json);

    return 0;
}

}
