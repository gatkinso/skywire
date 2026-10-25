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

// Fuzzes the generated C deserializer (SW wire format) with the ring message,
// which has every native type, both byte orders, strings, a nested message
// and signed and unsigned bitfields packed lsb and msb first.
//
// Properties, for any input:
//   - shorter than the message: _Deserialize fails and consumes nothing;
//   - otherwise it succeeds and consumes exactly _Size() bytes, and
//     _Serialize writes those bytes back unchanged. Every bitfield unit in
//     the ring message is full, so no input bit is dropped.

#include <cstring>
#include <vector>

#include "fuzz_check.h"
#include "fuzz_cstream.h"
#include "c/RingyMcRingface.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t message_size = RingyMcRingface_Size();

    std::vector<char> storage;
    Cstream in = fuzz::input_stream(data, size, storage);

    RingyMcRingface message;
    std::memset(&message, 0, sizeof(message));
    const int result = RingyMcRingface_Deserialize(&message, &in);

    if (size < message_size)
    {
        FUZZ_CHECK(result != 0);
        FUZZ_CHECK(cstream_available(&in) == size);
    }
    else
    {
        FUZZ_CHECK(result == 0);
        FUZZ_CHECK(cstream_available(&in) == size - message_size);

        Cstream out;
        FUZZ_CHECK(cstream_init(&out, 0) == 0);
        FUZZ_CHECK(RingyMcRingface_Serialize(&message, &out) == 0);
        FUZZ_CHECK(cstream_available(&out) == message_size);
        FUZZ_CHECK(std::memcmp(out.head_, data, message_size) == 0);
        cstream_fini(&out);
    }

    return 0;
}
