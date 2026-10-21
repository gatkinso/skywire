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

// Decoding protobuf (PZ) input that a real protobuf peer may send, but which
// the generated PZRoundTrip/PZWire tests never produce: fields out of order,
// repeated, unknown, of an unexpected wire type, short strings, bad bytes.

#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <string>
#include <protozero/pbf_writer.hpp>
#include "ProtoMcProtoface.h"

using testspace::ProtoMcProtoface;

namespace
{

// Decodes bytes through a PZ-mode stream, as a caller would.
bool decode(const std::string& bytes, ProtoMcProtoface& out, size_t* left = nullptr)
{
    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    stream.pz_buffer() = bytes;
    bool ok = out.deserialize(stream);
    if (left)
    {
        *left = stream.byte_size();
    }
    return ok;
}

}

TEST(PZDecode, FieldsInAnyOrder)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_int32(2048, -7);
    pbf.add_int32(1, 'x');

    ProtoMcProtoface out;
    size_t left = 1;
    ASSERT_TRUE(decode(bytes, out, &left));
    ASSERT_EQ(out.Get_h_myint32_2048(), -7);
    ASSERT_EQ(out.Get_a_char1(), 'x');
    ASSERT_EQ(left, 0u);
}

TEST(PZDecode, LastRepeatedFieldWins)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(2047, 1);
    pbf.add_uint32(2047, 2);

    ProtoMcProtoface out;
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(out.Get_g_myuint32_2047(), 2u);
}

TEST(PZDecode, MissingFieldsAreZero)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(15, 9);

    ProtoMcProtoface out;
    out.Set_h_myint32_2048(42);
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(out.Get_b_uchar15(), 9);
    ASSERT_EQ(out.Get_h_myint32_2048(), 0);
}

TEST(PZDecode, UnknownFieldsAreSkipped)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(2, 1);
    pbf.add_string(3, "unknown");
    pbf.add_fixed32(4, 5);
    pbf.add_fixed64(5, 6);
    pbf.add_int32(1, 'y');

    ProtoMcProtoface out;
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(out.Get_a_char1(), 'y');
}

TEST(PZDecode, UnexpectedWireTypeIsSkipped)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_string(1, "not a varint");
    pbf.add_fixed64(18999, 1);
    pbf.add_int32(17, 3);

    ProtoMcProtoface out;
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(out.Get_a_char1(), 0);
    ASSERT_EQ(out.Get_k_myfloat18999(), 0.0f);
    ASSERT_EQ(out.Get_d_myint8_17(), 3);
}

TEST(PZDecode, WideValuesAreNarrowed)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_int32(17, 300);
    pbf.add_uint32(100, 70000);

    ProtoMcProtoface out;
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(out.Get_d_myint8_17(), static_cast<int8_t>(300));
    ASSERT_EQ(out.Get_e_myuint16_100(), static_cast<uint16_t>(70000));
}

TEST(PZDecode, ShortStringIsZeroPadded)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_string(536870911, "ab");

    ProtoMcProtoface out;
    out.Set_n_longstring536870911("0123456789abcdef");
    ASSERT_TRUE(decode(bytes, out));

    const char want[16] = { 'a', 'b' };
    ASSERT_EQ(0, std::memcmp(out.Get_n_longstring536870911(), want, sizeof(want)));
}

TEST(PZDecode, FullLengthStringFits)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_string(20001, "abcd");

    ProtoMcProtoface out;
    ASSERT_TRUE(decode(bytes, out));
    ASSERT_EQ(0, std::memcmp(out.Get_m_shortstring20001(), "abcd", 4));
}

TEST(PZDecode, LongStringFailsAndChangesNothing)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_int32(1, 'z');
    pbf.add_string(20001, "abcde");

    ProtoMcProtoface out;
    out.Set_h_myint32_2048(42);
    size_t left = 0;
    ASSERT_FALSE(decode(bytes, out, &left));
    ASSERT_EQ(out.Get_a_char1(), 0);
    ASSERT_EQ(out.Get_h_myint32_2048(), 42);
    ASSERT_EQ(left, bytes.size());
}

TEST(PZDecode, TruncatedVarintFailsAndChangesNothing)
{
    // Field 1 (varint) whose value has its continuation bit set, then ends.
    const std::string bytes("\x08\x80", 2);

    ProtoMcProtoface out;
    out.Set_a_char1('q');
    size_t left = 0;
    ASSERT_FALSE(decode(bytes, out, &left));
    ASSERT_EQ(out.Get_a_char1(), 'q');
    ASSERT_EQ(left, bytes.size());
}

TEST(PZDecode, NegativeZeroIsSent)
{
    ProtoMcProtoface in;
    in.Set_l_mydouble20000(-0.0);

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_LT(0u, stream.byte_size());

    ProtoMcProtoface out;
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_TRUE(std::signbit(out.Get_l_mydouble20000()));
}
