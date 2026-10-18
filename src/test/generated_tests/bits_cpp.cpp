//
//   Copyright (C) 2026 Skyloupe, LLC
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

// Bitfield members against wire bytes worked out by hand, not by wiregen's
// own encoder (which the generated Wire tests use), so a packing mistake
// shared by the generators and the encoder still shows up.

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "CBitsyMcBitface.h"
#include "BitsyMcBitface.h"
#include "BitsyMsb.h"

namespace
{

// Units in wire order (see BitsyMcBitface.json):
//   a:4 b:3 c:1 (u8) | d:5 e:6 (u16 BE) | f:7 g:13 h:12 (i32 LE) | i_plain (u32 LE)
//   j:33 k:31 (u64 BE) | l:10 (u16 LE) | m:10 (u16 LE) | n:2 (u8) | o:3 p:1 (i8)
//   q:64 (i64 LE) | r:9 (i16 BE)
const std::vector<uint8_t> lsb_wire = {
    0xa9,                                            // 9 | 2<<4 | 1<<7
    0x05, 0x51,                                      // 0x11 | 0x2a<<5
    0xff, 0x91, 0x00, 0x80,                          // 0x7f | 0x123<<7 | 0x800<<20
    0x04, 0x03, 0x02, 0x01,                          // i_plain
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,  // 0x100000001 | 0x7fffffff<<33
    0xff, 0x03,                                      // 0x3ff
    0x00, 0x02,                                      // 0x200
    0x03,                                            // 3
    0x0c,                                            // 0b100 | 1<<3
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // -2
    0x01, 0x00,                                      // 0x100
};

// The same values with the first field in the high bits.
const std::vector<uint8_t> msb_wire = {
    0x95,                                            // 9<<4 | 2<<1 | 1
    0x8d, 0x40,                                      // 0x11<<11 | 0x2a<<5
    0x00, 0x38, 0x12, 0xfe,                          // 0x7f<<25 | 0x123<<12 | 0x800
    0x04, 0x03, 0x02, 0x01,
    0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,  // 0x100000001<<31 | 0x7fffffff
    0xc0, 0xff,                                      // 0x3ff<<6
    0x00, 0x80,                                      // 0x200<<6
    0xc0,                                            // 3<<6
    0x90,                                            // 0b100<<5 | 1<<4
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x80, 0x00,                                      // 0x100<<7
};

template <typename T>
void set_cpp(T& msg)
{
    msg.Set_a_u8_4(9);
    msg.Set_b_u8_3(2);
    msg.Set_c_u8_1(1);
    msg.Set_d_u16_5(0x11);
    msg.Set_e_u16_6(0x2a);
    msg.Set_f_i32_7(-1);
    msg.Set_g_i32_13(0x123);
    msg.Set_h_i32_12(-2048);
    msg.Set_i_plain(0x01020304);
    msg.Set_j_u64_33(0x100000001ull);
    msg.Set_k_u64_31(0x7fffffff);
    msg.Set_l_u16_10(0x3ff);
    msg.Set_m_u16_10(0x200);
    msg.Set_n_u8_2(3);
    msg.Set_o_i8_3(-4);
    msg.Set_p_i8_1(-1);
    msg.Set_q_i64_64(-2);
    msg.Set_r_i16_9(-256);
}

template <typename T>
std::vector<uint8_t> cpp_bytes(const T& msg)
{
    skyloupe::Stream str;
    EXPECT_TRUE(msg.serialize(str));
    return std::vector<uint8_t>(str.data(), str.data() + str.byte_size());
}

}

TEST(Bitfields, CppLsbWire)
{
    testspace::BitsyMcBitface msg;
    set_cpp(msg);
    ASSERT_EQ(lsb_wire, cpp_bytes(msg));
    ASSERT_EQ(lsb_wire.size(), msg.size());
}

TEST(Bitfields, CppMsbWire)
{
    testspace::BitsyMsb msg;
    set_cpp(msg);
    ASSERT_EQ(msb_wire, cpp_bytes(msg));
}

TEST(Bitfields, CMatchesCpp)
{
    CBitsyMcBitface s;
    std::memset(&s, 0, sizeof(s));
    s.a_u8_4 = 9;
    s.b_u8_3 = 2;
    s.c_u8_1 = 1;
    s.d_u16_5 = 0x11;
    s.e_u16_6 = 0x2a;
    s.f_i32_7 = -1;
    s.g_i32_13 = 0x123;
    s.h_i32_12 = -2048;
    s.i_plain = 0x01020304;
    s.j_u64_33 = 0x100000001ull;
    s.k_u64_31 = 0x7fffffff;
    s.l_u16_10 = 0x3ff;
    s.m_u16_10 = 0x200;
    s.n_u8_2 = 3;
    s.o_i8_3 = -4;
    s.p_i8_1 = -1;
    s.q_i64_64 = -2;
    s.r_i16_9 = -256;

    Cstream cs;
    cstream_init(&cs, 128);
    ASSERT_EQ(0, CBitsyMcBitface_Serialize(&s, &cs));
    ASSERT_EQ(lsb_wire.size(), CBitsyMcBitface_Size());
    ASSERT_EQ(lsb_wire, std::vector<uint8_t>(cs.head_, cs.head_ + cstream_available(&cs)));

    CBitsyMcBitface back;
    std::memset(&back, 0x5a, sizeof(back));
    ASSERT_EQ(0, CBitsyMcBitface_Deserialize(&back, &cs));
    ASSERT_EQ(1, CBitsyMcBitface_Equals(&s, &back));
    cstream_fini(&cs);
}

// Signed fields come back sign-extended, unsigned ones as written.
TEST(Bitfields, CppDecodesMsb)
{
    skyloupe::Stream str;
    str.write(msb_wire.data(), msb_wire.size());
    testspace::BitsyMsb msg;
    ASSERT_TRUE(msg.deserialize(str));
    ASSERT_EQ(9, msg.Get_a_u8_4());
    ASSERT_EQ(0x2a, msg.Get_e_u16_6());
    ASSERT_EQ(-1, msg.Get_f_i32_7());
    ASSERT_EQ(0x123, msg.Get_g_i32_13());
    ASSERT_EQ(-2048, msg.Get_h_i32_12());
    ASSERT_EQ(0x100000001ull, msg.Get_j_u64_33());
    ASSERT_EQ(-4, msg.Get_o_i8_3());
    ASSERT_EQ(-1, msg.Get_p_i8_1());
    ASSERT_EQ(-2, msg.Get_q_i64_64());
    ASSERT_EQ(-256, msg.Get_r_i16_9());
}

// Padding bits are ignored on read: setting every unused bit changes nothing.
TEST(Bitfields, PaddingIgnored)
{
    std::vector<uint8_t> wire = lsb_wire;
    wire[1] |= 0xf8;   // d/e unit (BE): bits 11-15
    wire[20] |= 0xfc;  // l unit (LE): bits 10-15
    wire[22] |= 0xfc;  // m unit
    wire[23] |= 0xfc;  // n unit: bits 2-7
    wire[24] |= 0xf0;  // o/p unit: bits 4-7
    wire[33] |= 0xfe;  // r unit (BE): bits 9-15

    skyloupe::Stream str;
    str.write(wire.data(), wire.size());
    testspace::BitsyMcBitface got, want;
    set_cpp(want);
    ASSERT_TRUE(got.deserialize(str));
    ASSERT_EQ(want, got);
}

// protobuf carries each field under its own tag, decoded through the setter's
// masking, so an out-of-range value from another encoder is cut to the field.
TEST(Bitfields, PzRoundTrip)
{
    testspace::BitsyMcBitface in, out;
    set_cpp(in);
    skyloupe::Stream str(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(str));
    ASSERT_TRUE(out.deserialize(str));
    ASSERT_EQ(in, out);
}

TEST(Bitfields, PzCutsOutOfRange)
{
    // field 1 (a_u8_4) = 0x1f, field 6 (f_i32_7) = 0x40 (-64 in 7 bits)
    skyloupe::Stream str(512, 512, skyloupe::Stream::PZ);
    str.pz_buffer() = std::string("\x08\x1f\x30\x40", 4);
    testspace::BitsyMcBitface msg;
    ASSERT_TRUE(msg.deserialize(str));
    ASSERT_EQ(0xf, msg.Get_a_u8_4());
    ASSERT_EQ(-64, msg.Get_f_i32_7());
}
