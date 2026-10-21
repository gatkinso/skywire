//
//   Copyright (C) 2017 Geoffrey Atkinson
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstring>
#include "cstream.h"

class CStreamUnitTest : public ::testing::Test
{
protected:
    CStreamUnitTest() {}
};

TEST_F(CStreamUnitTest, InitFini)
{
    Cstream cs;
    ASSERT_NO_THROW(cstream_init(&cs, 2048));
    ASSERT_NO_THROW(cstream_fini(&cs));
}

TEST_F(CStreamUnitTest, Write)
{
    Cstream cs;
    uint32_t ui32 = 0xCAFEBABE;
    const size_t strsize = sizeof(ui32) * 2;
    char buffer[strsize] = { 0 };

    std::memcpy(buffer, &ui32, sizeof(ui32));
    std::memcpy(buffer+sizeof(ui32), &ui32, sizeof(ui32));

    cstream_init(&cs, strsize);
    cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);
    cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);

    ASSERT_EQ(0, std::memcmp(cs.buffer_, buffer, strsize));

    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, Read)
{
    Cstream cs;
    uint32_t ui32_1 = 0xCAFEBABE;
    uint32_t ui32_2 = 0xFEEDBEEF;
    const size_t strsize = sizeof(uint32_t) * 2;
    char buffer_in[strsize] = { 0 };
    char buffer_out[strsize] = { 0 };

    std::memcpy(buffer_in, &ui32_1, sizeof(uint32_t));
    std::memcpy(buffer_in + sizeof(uint32_t), &ui32_2, sizeof(uint32_t));

    cstream_init(&cs, strsize);
    cstream_write(&cs, &ui32_1, sizeof(uint32_t), SW_LITTLE_ENDIAN_FLAG);
    cstream_write(&cs, &ui32_2, sizeof(uint32_t), SW_LITTLE_ENDIAN_FLAG);

    cstream_read(&cs, buffer_out, sizeof(uint32_t), SW_LITTLE_ENDIAN_FLAG);
    cstream_read(&cs, buffer_out + sizeof(uint32_t), sizeof(uint32_t), SW_LITTLE_ENDIAN_FLAG);

    ASSERT_EQ(0, std::memcmp(buffer_in, buffer_out, strsize));

    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, Grow)
{
    Cstream cs;
    uint32_t ui32 = 0xCAFEBABE;
    const size_t strsize = sizeof(ui32) * 2;
    char buffer[strsize+sizeof(ui32)] = { 0 };

    std::memcpy(buffer, &ui32, sizeof(ui32));
    std::memcpy(buffer + sizeof(ui32), &ui32, sizeof(ui32));
    std::memcpy(buffer + sizeof(ui32)*2, &ui32, sizeof(ui32));

    cstream_init(&cs, strsize);
    cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);
    cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);
    cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);

    ASSERT_EQ(0, std::memcmp(cs.buffer_, buffer, strsize));

    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, ManyWrite)
{
    Cstream cs;
    const size_t NUMITEMS = 100000;
    uint32_t ui32 = 0xCAFEBABE;
    const size_t strsize = sizeof(ui32) * NUMITEMS;
    char buffer[strsize] = { 0 };

    for (int i = 0; i < NUMITEMS; i++)
    {
        std::memcpy(buffer + (sizeof(ui32)*i), &ui32, sizeof(ui32));
    }

    cstream_init(&cs, 512);

    for (int i = 0; i < NUMITEMS; i++)
    {
        cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);
    }

    ASSERT_EQ(0, std::memcmp(cs.buffer_, buffer, strsize));

    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, ManyRead)
{
    Cstream cs;
    const size_t NUMITEMS = 100000;
    uint32_t ui32 = 0xCAFEBABE;
    const size_t strsize = sizeof(ui32) * NUMITEMS;
    char buffer_in[strsize] = { 0 };
    char buffer_out[strsize] = { 0 };

    for (int i = 0; i < NUMITEMS; i++)
    {
        std::memcpy(buffer_in + (sizeof(ui32)*i), &ui32, sizeof(ui32));
    }

    cstream_init(&cs, 1024*64);

    for (int i = 0; i < NUMITEMS; i++)
    {
        cstream_write(&cs, &ui32, sizeof(ui32), SW_LITTLE_ENDIAN_FLAG);
    }

    for (int i = 0; i < NUMITEMS; i++)
    {
        cstream_read(&cs, buffer_out + (sizeof(uint32_t)*i), sizeof(uint32_t), SW_LITTLE_ENDIAN_FLAG);
    }

    ASSERT_EQ(0, std::memcmp(buffer_in, buffer_out, strsize));

    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, NullArguments)
{
    Cstream cs;
    uint32_t v = 1;

    ASSERT_EQ(-1, cstream_init(NULL, 16));
    ASSERT_NO_THROW(cstream_fini(NULL));
    ASSERT_EQ(-1, cstream_write(NULL, &v, sizeof(v), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(-1, cstream_read(NULL, &v, sizeof(v), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(0u, cstream_available(NULL));

    ASSERT_EQ(0, cstream_init(&cs, 16));
    ASSERT_EQ(-1, cstream_write(&cs, NULL, sizeof(v), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(0u, cstream_available(&cs));
    cstream_fini(&cs);
    ASSERT_TRUE(cs.buffer_ == NULL);
    ASSERT_NO_THROW(cstream_fini(&cs));
}

TEST_F(CStreamUnitTest, ZeroSizeInitUsesDefault)
{
    Cstream cs;
    uint32_t v = 0xCAFEBABE, out = 0;

    ASSERT_EQ(0, cstream_init(&cs, 0));
    ASSERT_EQ((size_t)DEF_CSTREAM_SIZE, cs.size_);
    ASSERT_EQ(0, cstream_write(&cs, &v, sizeof(v), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(0, cstream_read(&cs, &out, sizeof(out), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(v, out);
    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, ReadSpaceIsReusedNotLeaked)
{
    // Steady write/read traffic must not keep growing the buffer; the old
    // read-side compaction allocated a new buffer and leaked the old one.
    Cstream cs;
    ASSERT_EQ(0, cstream_init(&cs, 64));

    for (uint32_t i = 0; i < 100000; i++)
    {
        uint32_t out = 0;
        ASSERT_EQ(0, cstream_write(&cs, &i, sizeof(i), SW_BIG_ENDIAN_FLAG));
        ASSERT_EQ(0, cstream_read(&cs, &out, sizeof(out), SW_BIG_ENDIAN_FLAG));
        ASSERT_EQ(i, out);
    }

    ASSERT_EQ(64u, cs.size_);
    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, InterleavedReadWriteKeepsOrder)
{
    Cstream cs;
    uint32_t out = 0;
    ASSERT_EQ(0, cstream_init(&cs, 8));

    for (uint32_t i = 0; i < 4; i++)
    {
        ASSERT_EQ(0, cstream_write(&cs, &i, sizeof(i), SW_LITTLE_ENDIAN_FLAG));
    }
    ASSERT_EQ(0, cstream_read(&cs, &out, sizeof(out), SW_LITTLE_ENDIAN_FLAG));
    ASSERT_EQ(0u, out);

    // Forces compaction of the consumed bytes, then growth.
    for (uint32_t i = 4; i < 64; i++)
    {
        ASSERT_EQ(0, cstream_write(&cs, &i, sizeof(i), SW_LITTLE_ENDIAN_FLAG));
    }
    for (uint32_t i = 1; i < 64; i++)
    {
        ASSERT_EQ(0, cstream_read(&cs, &out, sizeof(out), SW_LITTLE_ENDIAN_FLAG));
        ASSERT_EQ(i, out);
    }
    ASSERT_EQ(0u, cstream_available(&cs));
    cstream_fini(&cs);
}


TEST_F(CStreamUnitTest, ClaimAndConsume)
{
    Cstream cs;
    const char bytes[] = { 10, 20, 30, 40, 50, 60 };
    ASSERT_EQ(0, cstream_init(&cs, 4));

    // A claim counts as written, growing the buffer when needed.
    char* p = cstream_claim(&cs, sizeof(bytes));
    ASSERT_TRUE(p != NULL);
    std::memcpy(p, bytes, sizeof(bytes));
    ASSERT_EQ(sizeof(bytes), cstream_available(&cs));

    // A short consume takes nothing.
    ASSERT_TRUE(cstream_consume(&cs, sizeof(bytes) + 1) == NULL);
    ASSERT_EQ(sizeof(bytes), cstream_available(&cs));

    const char* q = cstream_consume(&cs, 2);
    ASSERT_TRUE(q != NULL);
    ASSERT_EQ(0, std::memcmp(q, bytes, 2));
    q = cstream_consume(&cs, 4);
    ASSERT_TRUE(q != NULL);
    ASSERT_EQ(0, std::memcmp(q, bytes + 2, 4));
    ASSERT_EQ(0u, cstream_available(&cs));

    ASSERT_TRUE(cstream_claim(&cs, 0) == NULL);
    ASSERT_TRUE(cstream_consume(&cs, 0) == NULL);
    ASSERT_TRUE(cstream_claim(NULL, 4) == NULL);
    ASSERT_TRUE(cstream_consume(NULL, 4) == NULL);
    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, ClaimReusesReadSpace)
{
    Cstream cs;
    const char bytes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    ASSERT_EQ(0, cstream_init(&cs, 8));

    std::memcpy(cstream_claim(&cs, 8), bytes, 8);
    ASSERT_TRUE(cstream_consume(&cs, 6) != NULL);
    char* p = cstream_claim(&cs, 4);
    ASSERT_TRUE(p != NULL);
    std::memcpy(p, bytes, 4);

    ASSERT_EQ(8u, cs.size_);
    const char want[] = { 7, 8, 1, 2, 3, 4 };
    ASSERT_EQ(sizeof(want), cstream_available(&cs));
    ASSERT_EQ(0, std::memcmp(cs.head_, want, sizeof(want)));
    cstream_fini(&cs);
}

TEST_F(CStreamUnitTest, PutGetByteOrder)
{
    char buf[8];
    const uint32_t u32 = 0x01020304;

    // Big and little endian are fixed on the wire; native is the host's order.
    ASSERT_EQ(buf + 4, sw_put(buf, &u32, sizeof(u32), SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(0, std::memcmp(buf, "\x01\x02\x03\x04", 4));
    sw_put(buf, &u32, sizeof(u32), SW_LITTLE_ENDIAN_FLAG);
    ASSERT_EQ(0, std::memcmp(buf, "\x04\x03\x02\x01", 4));
    sw_put(buf, &u32, sizeof(u32), SW_NATIVE_ENDIAN_FLAG);
    ASSERT_EQ(0, std::memcmp(buf, &u32, 4));

    const int16_t i16 = -2;
    sw_put(buf, &i16, sizeof(i16), SW_BIG_ENDIAN_FLAG);
    ASSERT_EQ(0, std::memcmp(buf, "\xff\xfe", 2));

    // 3.5f is 0x40600000; 3.5 is 0x400c000000000000.
    const float f = 3.5f;
    sw_put(buf, &f, sizeof(f), SW_BIG_ENDIAN_FLAG);
    ASSERT_EQ(0, std::memcmp(buf, "\x40\x60\x00\x00", 4));
    const double d = 3.5;
    sw_put(buf, &d, sizeof(d), SW_LITTLE_ENDIAN_FLAG);
    ASSERT_EQ(0, std::memcmp(buf, "\x00\x00\x00\x00\x00\x00\x0c\x40", 8));

    double d_out = 0;
    ASSERT_EQ(buf + 8, sw_get(buf, &d_out, sizeof(d_out), SW_LITTLE_ENDIAN_FLAG));
    ASSERT_EQ(3.5, d_out);

    uint64_t u64 = 0;
    std::memcpy(buf, "\x00\x00\x00\x02\xcb\x41\x78\x00", 8);
    sw_get(buf, &u64, sizeof(u64), SW_BIG_ENDIAN_FLAG);
    ASSERT_EQ(12000000000ull, u64);

    // Other sizes (char[N]) are copied without swapping.
    char text[3] = {};
    ASSERT_EQ(buf + 3, sw_put(buf, "sky", 3, SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(buf + 3, sw_get(buf, text, 3, SW_BIG_ENDIAN_FLAG));
    ASSERT_EQ(0, std::memcmp(text, "sky", 3));
}

TEST_F(CStreamUnitTest, SwapFunctions)
{
    char b2[] = { 1, 2 }, b4[] = { 1, 2, 3, 4 }, b8[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    sw_swap16(b2);
    sw_swap32(b4);
    sw_swap64(b8);
    ASSERT_EQ(0, std::memcmp(b2, "\x02\x01", 2));
    ASSERT_EQ(0, std::memcmp(b4, "\x04\x03\x02\x01", 4));
    ASSERT_EQ(0, std::memcmp(b8, "\x08\x07\x06\x05\x04\x03\x02\x01", 8));
    ASSERT_NO_THROW(sw_swap32(NULL));
}
