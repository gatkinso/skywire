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

// Fuzzes the generated C++ deserializer (SW wire format) with the ring
// message. The properties are those of fuzz_c_sw.cpp: a short input fails
// and consumes nothing; any other consumes exactly size() bytes and
// serializes back to them.

#include <cstring>

#include "fuzz_check.h"
#include "cpp/RingyMcRingface.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t message_size = ring::RingyMcRingface::size();

    skyloupe::Stream in;
    FUZZ_CHECK(in.write(data, size) == size);

    ring::RingyMcRingface message;
    const bool result = message.deserialize(in);

    if (size < message_size)
    {
        FUZZ_CHECK(!result);
        FUZZ_CHECK(in.byte_size() == size);
    }
    else
    {
        FUZZ_CHECK(result);
        FUZZ_CHECK(in.byte_size() == size - message_size);

        skyloupe::Stream out;
        FUZZ_CHECK(message.serialize(out));
        FUZZ_CHECK(out.byte_size() == message_size);
        FUZZ_CHECK(std::memcmp(out.data(), data, message_size) == 0);
    }

    return 0;
}
