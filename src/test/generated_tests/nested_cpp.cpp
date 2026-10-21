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

// Nested messages with values set, which the generated tests leave all zero,
// two and three levels deep:
// both wire formats, the protobuf bytes protoc gives for the same values
// (NestyMcNestface.sample.txtpb), nested decode failures, and C (CNesty*,
// the same specs under other names) against C++.

#include <gtest/gtest.h>
#include <cstring>
#include <fstream>
#include <string>
#include <protozero/pbf_reader.hpp>
#include <protozero/pbf_writer.hpp>
#include "NestyMcNestface.h"
#include "CNestyMcNestface.h"
#include "NestyOuter.h"
#include "CNestyOuter.h"

using testspace::NestyInner;
using testspace::NestyMcNestface;
using testspace::NestyOuter;

namespace
{

// The values in NestyMcNestface.sample.txtpb.
NestyMcNestface sample()
{
    NestyInner first;
    first.Set_a_myint16_1(-3);
    first.Set_b_myuint32_2(70000);
    first.Set_c_mydouble3(0.5);
    first.Set_d_mystring4("nest");

    NestyInner second;
    second.Set_d_mystring4("eightchr");

    NestyMcNestface msg;
    msg.Set_a_myuint8_1(200);
    msg.Set_b_first2(first);
    msg.Set_c_myint64_3(-1);
    msg.Set_d_second2048(second);
    msg.Set_e_myfloat2049(1.5f);
    return msg;
}

// The values in NestyOuter.sample.txtpb: three levels, values at each.
NestyOuter outer_sample()
{
    NestyInner inner;
    inner.Set_a_myint16_1(7);
    inner.Set_d_mystring4("deep");

    NestyOuter msg;
    msg.Set_a_myuint16_1(60000);
    msg.Set_b_middle2(sample());
    msg.Set_c_inner3(inner);
    msg.Set_d_mydouble4(-2.25);
    return msg;
}

// A sample's "# expected:" hex, which Proto3Wire checks protoc produces.
std::string sample_bytes(const char* path = NESTY_SAMPLE)
{
    std::ifstream in(path);
    std::string line;
    const std::string prefix = "# expected: ";
    while (std::getline(in, line))
    {
        if (line.compare(0, prefix.size(), prefix) == 0)
        {
            std::string bytes;
            for (size_t i = prefix.size(); i + 1 < line.size(); i += 2)
            {
                bytes.push_back(static_cast<char>(std::stoi(line.substr(i, 2), nullptr, 16)));
            }
            return bytes;
        }
    }
    return {};
}

template <typename T>
std::string pz_bytes(const T& msg)
{
    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    EXPECT_TRUE(msg.serialize(stream));
    return std::string(stream.data(), stream.byte_size());
}

template <typename T>
bool pz_decode(const std::string& bytes, T& out)
{
    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    stream.pz_buffer() = bytes;
    return out.deserialize(stream);
}

}

TEST(Nested, PZMatchesProtoc)
{
    const std::string want = sample_bytes();
    ASSERT_FALSE(want.empty()) << "no # expected: line in " << NESTY_SAMPLE;
    ASSERT_EQ(pz_bytes(sample()), want);
}

TEST(Nested, PZRoundTrip)
{
    NestyMcNestface out;
    ASSERT_TRUE(pz_decode(pz_bytes(sample()), out));
    ASSERT_EQ(out, sample());
}

// Each nested message is an embedded message on its own field number.
TEST(Nested, PZInnerIsLengthDelimited)
{
    const std::string bytes = pz_bytes(sample());
    protozero::pbf_reader pbf{bytes};

    int found = 0;
    while (pbf.next())
    {
        const auto tag = pbf.tag();
        if (tag == 2 || tag == 2048)
        {
            ASSERT_EQ(pbf.wire_type(), protozero::pbf_wire_type::length_delimited);
            NestyInner inner;
            ASSERT_TRUE(inner.deserialize_pb(pbf.get_message()));
            ASSERT_EQ(inner, tag == 2 ? sample().Get_b_first2() : sample().Get_d_second2048());
            found++;
        }
        else
        {
            pbf.skip();
        }
    }
    ASSERT_EQ(found, 2);
}

// An all-zero nested message is left out, like any zero field.
TEST(Nested, PZZeroInnerLeftOut)
{
    NestyMcNestface msg;
    msg.Set_a_myuint8_1(1);
    ASSERT_EQ(pz_bytes(msg), std::string("\x08\x01", 2));
}

// A nested field seen twice merges, as protobuf requires: later fields win,
// fields only in the earlier one stay.
TEST(Nested, PZRepeatedInnerMerges)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    {
        protozero::pbf_writer sub{pbf, 2};
        sub.add_int32(1, 5);
        sub.add_uint32(2, 6);
    }
    {
        protozero::pbf_writer sub{pbf, 2};
        sub.add_uint32(2, 7);
    }

    NestyMcNestface out;
    ASSERT_TRUE(pz_decode(bytes, out));
    ASSERT_EQ(out.Get_b_first2().Get_a_myint16_1(), 5);
    ASSERT_EQ(out.Get_b_first2().Get_b_myuint32_2(), 7u);
}

// A bad inner field fails the whole message and leaves the destination alone.
TEST(Nested, PZInnerFailureFails)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(1, 9);
    {
        protozero::pbf_writer sub{pbf, 2048};
        sub.add_bytes(4, "ninechars");
    }

    NestyMcNestface out = sample();
    ASSERT_FALSE(pz_decode(bytes, out));
    ASSERT_EQ(out, sample());
}

// A nested field with the wrong wire type is skipped like any other.
TEST(Nested, PZInnerWrongWireTypeSkipped)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(2, 42);
    pbf.add_uint32(1, 9);

    NestyMcNestface out;
    ASSERT_TRUE(pz_decode(bytes, out));
    ASSERT_EQ(out.Get_a_myuint8_1(), 9);
    ASSERT_EQ(out.Get_b_first2(), NestyInner());
}

TEST(Nested, SWRoundTrip)
{
    skyloupe::Stream stream;
    ASSERT_TRUE(sample().serialize(stream));
    ASSERT_EQ(stream.byte_size(), NestyMcNestface::size());

    NestyMcNestface out;
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(out, sample());
    ASSERT_EQ(stream.byte_size(), 0u);
}

// In SW mode a nested message is its own serialization, inline, with its own
// members' byte orders.
TEST(Nested, SWInnerIsInline)
{
    const NestyMcNestface msg = sample();

    skyloupe::Stream want;
    want.serialize(msg.Get_a_myuint8_1(), skyloupe::Endianness::Big);
    ASSERT_TRUE(msg.Get_b_first2().serialize(want));
    want.serialize(msg.Get_c_myint64_3(), skyloupe::Endianness::Little);
    ASSERT_TRUE(msg.Get_d_second2048().serialize(want));
    want.serialize(msg.Get_e_myfloat2049(), skyloupe::Endianness::Big);

    skyloupe::Stream got;
    ASSERT_TRUE(msg.serialize(got));
    ASSERT_EQ(got.byte_size(), want.byte_size());
    ASSERT_EQ(0, std::memcmp(got.data(), want.data(), want.byte_size()));

    ASSERT_EQ(NestyInner::size(), 2u + 4u + 8u + 8u);
    ASSERT_EQ(NestyMcNestface::size(), 1u + 2 * NestyInner::size() + 8u + 4u);
}

// C writes the same SW bytes as C++, and reads them back.
TEST(Nested, CMatchesCpp)
{
    CNestyMcNestface c;
    std::memset(&c, 0, sizeof(c));
    c.a_myuint8_1 = 200;
    c.b_first2.a_myint16_1 = -3;
    c.b_first2.b_myuint32_2 = 70000;
    c.b_first2.c_mydouble3 = 0.5;
    std::memcpy(c.b_first2.d_mystring4, "nest", 4);
    c.c_myint64_3 = -1;
    std::memcpy(c.d_second2048.d_mystring4, "eightchr", 8);
    c.e_myfloat2049 = 1.5f;

    Cstream cs;
    ASSERT_EQ(0, cstream_init(&cs, 128));
    ASSERT_EQ(0, CNestyMcNestface_Serialize(&c, &cs));
    ASSERT_EQ(CNestyMcNestface_Size(), NestyMcNestface::size());

    skyloupe::Stream want;
    ASSERT_TRUE(sample().serialize(want));
    ASSERT_EQ(cstream_available(&cs), want.byte_size());
    ASSERT_EQ(0, std::memcmp(cs.head_, want.data(), want.byte_size()));

    CNestyMcNestface back;
    std::memset(&back, 0, sizeof(back));
    ASSERT_EQ(0, CNestyMcNestface_Deserialize(&back, &cs));
    ASSERT_EQ(1, CNestyMcNestface_Equals(&c, &back));
    back.d_second2048.d_mystring4[0] = 'x';
    ASSERT_EQ(0, CNestyMcNestface_Equals(&c, &back));

    cstream_fini(&cs);
}

// Three levels: NestyOuter holds a NestyMcNestface, which holds NestyInner.
TEST(Nested, ThreeLevelPZMatchesProtoc)
{
    const std::string want = sample_bytes(NESTY_OUTER_SAMPLE);
    ASSERT_FALSE(want.empty()) << "no # expected: line in " << NESTY_OUTER_SAMPLE;
    ASSERT_EQ(pz_bytes(outer_sample()), want);
}

TEST(Nested, ThreeLevelPZRoundTrip)
{
    NestyOuter out;
    ASSERT_TRUE(pz_decode(pz_bytes(outer_sample()), out));
    ASSERT_EQ(out, outer_sample());
    ASSERT_EQ(std::string(out.Get_b_middle2().Get_d_second2048().Get_d_mystring4(), 8), "eightchr");
}

// A bad field at the third level fails the whole message and leaves the
// destination alone.
TEST(Nested, ThreeLevelPZDeepFailureFails)
{
    std::string bytes;
    protozero::pbf_writer pbf{bytes};
    pbf.add_uint32(1, 9);
    {
        protozero::pbf_writer middle{pbf, 2};
        middle.add_uint32(1, 5);
        {
            protozero::pbf_writer inner{middle, 2048};
            inner.add_bytes(4, "ninechars");
        }
    }

    NestyOuter out = outer_sample();
    ASSERT_FALSE(pz_decode(bytes, out));
    ASSERT_EQ(out, outer_sample());
}

TEST(Nested, ThreeLevelSWRoundTrip)
{
    skyloupe::Stream stream;
    ASSERT_TRUE(outer_sample().serialize(stream));
    ASSERT_EQ(stream.byte_size(), NestyOuter::size());

    NestyOuter out;
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(out, outer_sample());
    ASSERT_EQ(stream.byte_size(), 0u);
}

TEST(Nested, ThreeLevelSWInline)
{
    const NestyOuter msg = outer_sample();

    skyloupe::Stream want;
    want.serialize(msg.Get_a_myuint16_1(), skyloupe::Endianness::Little);
    ASSERT_TRUE(msg.Get_b_middle2().serialize(want));
    ASSERT_TRUE(msg.Get_c_inner3().serialize(want));
    want.serialize(msg.Get_d_mydouble4(), skyloupe::Endianness::Big);

    skyloupe::Stream got;
    ASSERT_TRUE(msg.serialize(got));
    ASSERT_EQ(got.byte_size(), want.byte_size());
    ASSERT_EQ(0, std::memcmp(got.data(), want.data(), want.byte_size()));
    ASSERT_EQ(NestyOuter::size(), 2u + NestyMcNestface::size() + NestyInner::size() + 8u);
}

// C writes the same three-level SW bytes as C++, and reads them back.
TEST(Nested, ThreeLevelCMatchesCpp)
{
    CNestyOuter c;
    std::memset(&c, 0, sizeof(c));
    c.a_myuint16_1 = 60000;
    c.b_middle2.a_myuint8_1 = 200;
    c.b_middle2.b_first2.a_myint16_1 = -3;
    c.b_middle2.b_first2.b_myuint32_2 = 70000;
    c.b_middle2.b_first2.c_mydouble3 = 0.5;
    std::memcpy(c.b_middle2.b_first2.d_mystring4, "nest", 4);
    c.b_middle2.c_myint64_3 = -1;
    std::memcpy(c.b_middle2.d_second2048.d_mystring4, "eightchr", 8);
    c.b_middle2.e_myfloat2049 = 1.5f;
    c.c_inner3.a_myint16_1 = 7;
    std::memcpy(c.c_inner3.d_mystring4, "deep", 4);
    c.d_mydouble4 = -2.25;

    Cstream cs;
    ASSERT_EQ(0, cstream_init(&cs, 128));
    ASSERT_EQ(0, CNestyOuter_Serialize(&c, &cs));
    ASSERT_EQ(CNestyOuter_Size(), NestyOuter::size());

    skyloupe::Stream want;
    ASSERT_TRUE(outer_sample().serialize(want));
    ASSERT_EQ(cstream_available(&cs), want.byte_size());
    ASSERT_EQ(0, std::memcmp(cs.head_, want.data(), want.byte_size()));

    CNestyOuter back;
    std::memset(&back, 0, sizeof(back));
    ASSERT_EQ(0, CNestyOuter_Deserialize(&back, &cs));
    ASSERT_EQ(1, CNestyOuter_Equals(&c, &back));
    back.b_middle2.b_first2.d_mystring4[3] = 'x';
    ASSERT_EQ(0, CNestyOuter_Equals(&c, &back));

    cstream_fini(&cs);
}
