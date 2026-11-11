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

// The generated C and C++ JSON, XML and BSON code, against each other, with
// the ring message. Each input is read as all three forms, by both. For each:
//
//   - C accepts it exactly when C++ does;
//   - when they accept it, C writes back exactly the bytes C++ writes back,
//     so they read the same values, and write them the same way;
//   - when C refuses it, the C message is unchanged;
//   - C's length (the snprintf contract) matches what it writes.

#include <cstring>
#include <string>
#include <vector>

#include "fuzz_check.h"
#include "c_text/RingyMcRingface.h"
#include "cpp_text/RingyMcRingface.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    return 0;
}

namespace
{

// A C message that is not all zeros, to show a failed read leaves it alone;
// before is a byte-for-byte copy (padding too, which assignment need not copy).
void start(RingyMcRingface* m, RingyMcRingface* before)
{
    std::memset(m, 0, sizeof(*m));
    m->u8 = 7;
    m->i64_be = -3;
    m->f64_le = 0.5;
    std::memcpy(m->str_exact, "abc", 3);
    m->inner.i32_be = 42;
    std::memcpy(before, m, sizeof(*m));
}

bool same(const RingyMcRingface& a, const RingyMcRingface& b)
{
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}

std::string c_json(const RingyMcRingface& m)
{
    const size_t n = RingyMcRingface_ToJson(&m, nullptr, 0);
    std::vector<char> buf(n + 1);
    FUZZ_CHECK(RingyMcRingface_ToJson(&m, buf.data(), buf.size()) == n);
    FUZZ_CHECK(buf[n] == '\0');
    return std::string(buf.data(), n);
}

std::string c_xml(const RingyMcRingface& m)
{
    const size_t n = RingyMcRingface_ToXml(&m, nullptr, 0);
    std::vector<char> buf(n + 1);
    FUZZ_CHECK(RingyMcRingface_ToXml(&m, buf.data(), buf.size()) == n);
    FUZZ_CHECK(buf[n] == '\0');
    return std::string(buf.data(), n);
}

std::string c_bson(const RingyMcRingface& m)
{
    const size_t n = RingyMcRingface_ToBson(&m, nullptr, 0);
    std::vector<uint8_t> buf(n);
    FUZZ_CHECK(RingyMcRingface_ToBson(&m, buf.data(), buf.size()) == n);
    return std::string(buf.begin(), buf.end());
}

}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const char* text = reinterpret_cast<const char*>(data);
    const std::string_view view(text, size);

    {
        RingyMcRingface c, before;
        start(&c, &before);
        ring::RingyMcRingface cpp;
        const bool c_ok = RingyMcRingface_FromJson(&c, text, size) == 0;
        FUZZ_CHECK(c_ok == cpp.from_json(view));
        if (c_ok)
            FUZZ_CHECK(c_json(c) == cpp.to_json());
        else
            FUZZ_CHECK(same(c, before));
    }

    {
        RingyMcRingface c, before;
        start(&c, &before);
        ring::RingyMcRingface cpp;
        const bool c_ok = RingyMcRingface_FromXml(&c, text, size) == 0;
        FUZZ_CHECK(c_ok == cpp.from_xml(view));
        if (c_ok)
            FUZZ_CHECK(c_xml(c) == cpp.to_xml());
        else
            FUZZ_CHECK(same(c, before));
    }

    {
        RingyMcRingface c, before;
        start(&c, &before);
        ring::RingyMcRingface cpp;
        const bool c_ok = RingyMcRingface_FromBson(&c, data, size) == 0;
        FUZZ_CHECK(c_ok == cpp.from_bson(view));
        if (c_ok)
            FUZZ_CHECK(c_bson(c) == cpp.to_bson());
        else
            FUZZ_CHECK(same(c, before));
    }

    return 0;
}
