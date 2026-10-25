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

// Differential fuzzing of the generated C and C++ deserializers (SW wire
// format): both decode the same bytes of the ring message.
//
// Properties, for any input:
//   - both succeed or both fail;
//   - when they succeed, every member has the same value in both (floats
//     compared bit for bit, so NaNs count), and both serialize to the same
//     bytes.
// The round-trip targets would miss a member that both languages read and
// write wrongly but symmetrically; this one catches it when only one does.

#include <cstring>
#include <vector>

#include "fuzz_check.h"
#include "fuzz_cstream.h"
#include "c/RingyMcRingface.h"
#include "cpp/RingyMcRingface.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
    return 0;
}

namespace
{

template <typename T>
bool same_bits(const T& left, const T& right)
{
    return std::memcmp(&left, &right, sizeof(T)) == 0;
}

// Members of RingyInner, as (type, name).
#define RING_INNER_MEMBERS(X) \
    X(uint16_t, u16_le)       \
    X(int32_t, i32_be)        \
    X(double, f64_le)         \
    X(int8_t, i8)             \
    X(int16_t, s5)            \
    X(int16_t, s11)

// Members of RingyMcRingface other than strings and inner.
#define RING_MEMBERS(X)   \
    X(uint8_t, u8)        \
    X(int8_t, i8)         \
    X(char, ch)           \
    X(unsigned char, uch) \
    X(uint16_t, u16_be)   \
    X(uint16_t, u16_le)   \
    X(int16_t, i16_be)    \
    X(int16_t, i16_le)    \
    X(uint32_t, u32_be)   \
    X(uint32_t, u32_le)   \
    X(int32_t, i32_be)    \
    X(int32_t, i32_le)    \
    X(uint64_t, u64_be)   \
    X(uint64_t, u64_le)   \
    X(int64_t, i64_be)    \
    X(int64_t, i64_le)    \
    X(float, f32_be)      \
    X(float, f32_le)      \
    X(double, f64_be)     \
    X(double, f64_le)     \
    X(uint8_t, bf_three)  \
    X(uint8_t, bf_five)   \
    X(uint16_t, bf_flag)  \
    X(uint16_t, bf_seven) \
    X(uint16_t, bf_byte)

void check_same(RingyMcRingface& c, const ring::RingyMcRingface& cpp)
{
#define CHECK_MEMBER(type, name)                                 \
    {                                                            \
        type value{};                                            \
        FUZZ_CHECK(Get_RingyMcRingface_##name(&c, &value) == 0); \
        FUZZ_CHECK(same_bits(value, cpp.Get_##name()));          \
    }
    RING_MEMBERS(CHECK_MEMBER)
#undef CHECK_MEMBER

    RingyInner c_inner;
    FUZZ_CHECK(Get_RingyMcRingface_inner(&c, &c_inner) == 0);
    const ring::RingyInner cpp_inner = cpp.Get_inner();
#define CHECK_INNER_MEMBER(type, name)                            \
    {                                                             \
        type value{};                                             \
        FUZZ_CHECK(Get_RingyInner_##name(&c_inner, &value) == 0); \
        FUZZ_CHECK(same_bits(value, cpp_inner.Get_##name()));     \
    }
    RING_INNER_MEMBERS(CHECK_INNER_MEMBER)
#undef CHECK_INNER_MEMBER

    char text[5];
    FUZZ_CHECK(Get_RingyInner_text(&c_inner, text) == 0);
    FUZZ_CHECK(std::memcmp(text, cpp_inner.Get_text(), sizeof(text)) == 0);

    char padded[16];
    FUZZ_CHECK(Get_RingyMcRingface_str_padded(&c, padded) == 0);
    FUZZ_CHECK(std::memcmp(padded, cpp.Get_str_padded(), sizeof(padded)) == 0);
    char exact[7];
    FUZZ_CHECK(Get_RingyMcRingface_str_exact(&c, exact) == 0);
    FUZZ_CHECK(std::memcmp(exact, cpp.Get_str_exact(), sizeof(exact)) == 0);
    char truncated[3];
    FUZZ_CHECK(Get_RingyMcRingface_str_truncated(&c, truncated) == 0);
    FUZZ_CHECK(std::memcmp(truncated, cpp.Get_str_truncated(), sizeof(truncated)) == 0);
}

}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FUZZ_CHECK(RingyMcRingface_Size() == ring::RingyMcRingface::size());

    std::vector<char> storage;
    Cstream c_in = fuzz::input_stream(data, size, storage);
    RingyMcRingface c_message;
    std::memset(&c_message, 0, sizeof(c_message));
    const bool c_ok = RingyMcRingface_Deserialize(&c_message, &c_in) == 0;

    skyloupe::Stream cpp_in;
    FUZZ_CHECK(cpp_in.write(data, size) == size);
    ring::RingyMcRingface cpp_message;
    const bool cpp_ok = cpp_message.deserialize(cpp_in);

    FUZZ_CHECK(c_ok == cpp_ok);
    if (!c_ok)
    {
        return 0;
    }

    check_same(c_message, cpp_message);

    Cstream c_out;
    FUZZ_CHECK(cstream_init(&c_out, 0) == 0);
    FUZZ_CHECK(RingyMcRingface_Serialize(&c_message, &c_out) == 0);
    skyloupe::Stream cpp_out;
    FUZZ_CHECK(cpp_message.serialize(cpp_out));
    FUZZ_CHECK(cstream_available(&c_out) == cpp_out.byte_size());
    FUZZ_CHECK(std::memcmp(c_out.head_, cpp_out.data(), cpp_out.byte_size()) == 0);
    cstream_fini(&c_out);

    return 0;
}
