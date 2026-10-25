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

// Fuzzes the generated C++ protobuf (PZ) decoder with the ring message. The
// input is any byte string: truncated varints, bad lengths, wrong wire types,
// unknown fields, repeated fields, oversized strings, nested garbage.
//
// Properties, for any input:
//   - deserialize() returns false, leaving the stream as it was, or true;
//   - a decoded message re-encodes, and that encoding decodes and re-encodes
//     to the same bytes (the encoder's output is canonical);
//   - it also round-trips through the SW wire format unchanged.

#include <cstring>
#include <string>

#include "fuzz_check.h"
#include "cpp/RingyMcRingface.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    return 0;
}

static std::string encode_pz(const ring::RingyMcRingface& message)
{
    skyloupe::Stream out(512, 512, skyloupe::Stream::PZ);
    FUZZ_CHECK(message.serialize(out));
    return out.pz_buffer();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const std::string input(reinterpret_cast<const char*>(data), size);

    skyloupe::Stream in(512, 512, skyloupe::Stream::PZ);
    in.pz_buffer() = input;

    ring::RingyMcRingface message;
    if (!message.deserialize(in))
    {
        FUZZ_CHECK(in.pz_buffer() == input);
        return 0;
    }

    const std::string encoded = encode_pz(message);

    skyloupe::Stream again(512, 512, skyloupe::Stream::PZ);
    again.pz_buffer() = encoded;
    ring::RingyMcRingface decoded;
    FUZZ_CHECK(decoded.deserialize(again));
    FUZZ_CHECK(encode_pz(decoded) == encoded);

    skyloupe::Stream sw;
    FUZZ_CHECK(message.serialize(sw));
    FUZZ_CHECK(sw.byte_size() == ring::RingyMcRingface::size());
    const std::string sw_bytes(sw.data(), sw.byte_size());

    ring::RingyMcRingface from_sw;
    FUZZ_CHECK(from_sw.deserialize(sw));
    FUZZ_CHECK(encode_pz(from_sw) == encoded);
    skyloupe::Stream sw_again;
    FUZZ_CHECK(from_sw.serialize(sw_again));
    FUZZ_CHECK(std::string(sw_again.data(), sw_again.byte_size()) == sw_bytes);

    return 0;
}
