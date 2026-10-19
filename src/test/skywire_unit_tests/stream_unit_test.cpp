//
//   Copyright (C) 2017 Skyloupe, LLC
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
#include <type_traits>
#include <vector>
#include "stream.h"

class StreamUnitTest : public ::testing::Test
{
protected:
    StreamUnitTest() {}
};

TEST_F(StreamUnitTest, Serialize)
{
    skyloupe::Stream ss;
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);

    ss.serialize(ui32, skyloupe::Endianness::Native);
    ss.serialize(floaty, skyloupe::Endianness::Native);

    ASSERT_EQ(ss.byte_size(), sizeof(ui32) + sizeof(floaty));
}

TEST_F(StreamUnitTest, SerDes)
{
    skyloupe::Stream ss;
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);
    float boaty = static_cast<float>(654.321);
    char coal = 'G';

    uint32_t ui32_out = 0;
    float floaty_out = static_cast<float>(0);
    float boaty_out = static_cast<float>(0);
    char coal_out = 0;

    ss.serialize(ui32, skyloupe::Endianness::Native);
    ss.serialize(floaty, skyloupe::Endianness::Native);
    ss.serialize(boaty, skyloupe::Endianness::Native);
    ss.serialize(coal, skyloupe::Endianness::Native);

    ASSERT_EQ(ss.byte_size(), sizeof(ui32) + sizeof(floaty) + sizeof(boaty) + sizeof(coal));

    ss.deserialize(ui32_out, skyloupe::Endianness::Native);
    ss.deserialize(floaty_out, skyloupe::Endianness::Native);
    ss.deserialize(boaty_out, skyloupe::Endianness::Native);
    ss.deserialize(coal_out, skyloupe::Endianness::Native);

    ASSERT_EQ(ss.byte_size(), 0);

    ASSERT_EQ(ui32, ui32_out);
    ASSERT_EQ(floaty, floaty_out);
    ASSERT_EQ(boaty, boaty_out);
    ASSERT_EQ(coal, coal_out);
}

TEST_F(StreamUnitTest, WriteBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss;
    uint64_t* big_array = new uint64_t[ARRAYSIZE];

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
    }

    size_t bytes = ss.write(big_array, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    bytes = ARRAYSIZE * sizeof(uint64_t);
    ASSERT_EQ(ss.byte_size(), bytes);
    
    delete [] big_array;
}

TEST_F(StreamUnitTest, WriteReadBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss;
    uint64_t* big_array = new uint64_t[ARRAYSIZE];
    uint64_t* big_array_out = new uint64_t[ARRAYSIZE];

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
        big_array_out[i] = 0;
    }

    size_t bytes = ss.write(big_array, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    bytes = ARRAYSIZE * sizeof(uint64_t);
    ASSERT_EQ(ss.byte_size(), bytes);

    bytes = ss.read(big_array_out, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        ASSERT_EQ(big_array_out[i], i);
    }

    delete [] big_array_out;
    delete [] big_array;
}

TEST_F(StreamUnitTest, WriteLittleGrows)
{
    skyloupe::Stream ss(512, 32);
    const size_t NUMWRITES = 100000;
    double boaty = 0.0;

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        size_t bytes = ss.write(&boaty, sizeof(boaty));
        ASSERT_EQ(bytes, sizeof(boaty));
        boaty = static_cast<double>(i);
    }

    size_t bytes = NUMWRITES * sizeof(boaty);
    ASSERT_EQ(ss.byte_size(), bytes);
}

TEST_F(StreamUnitTest, WriteReadLittleGrows)
{
    skyloupe::Stream ss(512, 32);
    const size_t NUMWRITES = 100000;
    double boaty = 0.0;
    double* array_out = new double[NUMWRITES];

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        array_out[i] = 0.0;
        boaty = static_cast<double>(i);
        size_t bytes = ss.write(&boaty, sizeof(boaty));
        ASSERT_EQ(bytes, sizeof(boaty));
    }

    size_t bytes = NUMWRITES * sizeof(boaty);
    ASSERT_EQ(ss.byte_size(), bytes);

    bytes = ss.read(array_out, NUMWRITES * sizeof(double));
    ASSERT_EQ(bytes, NUMWRITES * sizeof(double));

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        ASSERT_EQ(array_out[i], static_cast<double>(i));
    }

    delete [] array_out;
}

TEST_F(StreamUnitTest, SequentialSerDes)
{
    skyloupe::Stream ss(16 * 1024);
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);
    float boaty = static_cast<float>(654.321);
    char coal = 'G';

    size_t I = 254;
    size_t J = 1000;

    for (uint32_t i = 0; i < I; i++) {
        for(uint32_t j = 0; j < J; j++) {
            uint32_t ui32_out = i+j;
            float floaty_out = static_cast<float>(i+j);
            float boaty_out = static_cast<float>(i+j);
            char coal_out = static_cast<char>(i+j);

            ss.serialize(ui32, skyloupe::Endianness::Native);
            ss.serialize(floaty, skyloupe::Endianness::Native);
            ss.serialize(boaty, skyloupe::Endianness::Native);
            ss.serialize(coal, skyloupe::Endianness::Native);

            ss.deserialize(ui32_out, skyloupe::Endianness::Native);
            ss.deserialize(floaty_out, skyloupe::Endianness::Native);
            ss.deserialize(boaty_out, skyloupe::Endianness::Native);
            ss.deserialize(coal_out, skyloupe::Endianness::Native);

            ASSERT_EQ(ss.byte_size(), 0);
            ASSERT_EQ(ui32, ui32_out);
            ASSERT_EQ(floaty, floaty_out);
            ASSERT_EQ(boaty, boaty_out);
            ASSERT_EQ(coal, coal_out);
        }
    }
}

TEST_F(StreamUnitTest, InterleavedSerDes)
{
    skyloupe::Stream ss(16 * 1024);
    uint32_t total = 10000;
    uint32_t i = 0;
    uint32_t j = 0;

    for (i = 0; i < total/2; i++)
    {
        uint32_t ui32 = i;
        float floaty = static_cast<float>(i);
        float boaty = static_cast<float>(i);
        char coal = static_cast<char>(i);

        ss.serialize(ui32, skyloupe::Endianness::Native);
        ss.serialize(floaty, skyloupe::Endianness::Native);
        ss.serialize(boaty, skyloupe::Endianness::Native);
        ss.serialize(coal, skyloupe::Endianness::Native);
    }

    for (j = 0; j < total/4; j++) 
    {
        uint32_t ui32 = j;
        float floaty = static_cast<float>(j);
        float boaty = static_cast<float>(j);
        char coal = static_cast<char>(j);

        uint32_t ui32_out = 0;
        float floaty_out = 0;
        float boaty_out = 0;
        char coal_out = 0;

        ss.deserialize(ui32_out, skyloupe::Endianness::Native);
        ss.deserialize(floaty_out, skyloupe::Endianness::Native);
        ss.deserialize(boaty_out, skyloupe::Endianness::Native);
        ss.deserialize(coal_out, skyloupe::Endianness::Native);

        ASSERT_EQ(ui32, ui32_out);
        ASSERT_EQ(floaty, floaty_out);
        ASSERT_EQ(boaty, boaty_out);
        ASSERT_EQ(coal, coal_out);
    }

    for (; i < (3*total)/4; i++)
    {
        uint32_t ui32 = i;
        float floaty = static_cast<float>(i);
        float boaty = static_cast<float>(i);
        char coal = static_cast<char>(i);

        ss.serialize(ui32, skyloupe::Endianness::Native);
        ss.serialize(floaty, skyloupe::Endianness::Native);
        ss.serialize(boaty, skyloupe::Endianness::Native);
        ss.serialize(coal, skyloupe::Endianness::Native);
    }

    for (; j < i-2; j++)
    {
        uint32_t ui32 = j;
        float floaty = static_cast<float>(j);
        float boaty = static_cast<float>(j);
        char coal = static_cast<char>(j);

        uint32_t ui32_out = 0;
        float floaty_out = 0;
        float boaty_out = 0;
        char coal_out = 0;

        ss.deserialize(ui32_out, skyloupe::Endianness::Native);
        ss.deserialize(floaty_out, skyloupe::Endianness::Native);
        ss.deserialize(boaty_out, skyloupe::Endianness::Native);
        ss.deserialize(coal_out, skyloupe::Endianness::Native);

        ASSERT_EQ(ui32, ui32_out);
        ASSERT_EQ(floaty, floaty_out);
        ASSERT_EQ(boaty, boaty_out);
        ASSERT_EQ(coal, coal_out);
    }

    for (; i < total; i++)
    {
        uint32_t ui32 = i;
        float floaty = static_cast<float>(i);
        float boaty = static_cast<float>(i);
        char coal = static_cast<char>(i);

        ss.serialize(ui32, skyloupe::Endianness::Native);
        ss.serialize(floaty, skyloupe::Endianness::Native);
        ss.serialize(boaty, skyloupe::Endianness::Native);
        ss.serialize(coal, skyloupe::Endianness::Native);
    }

    for (; j < total; j++)
    {
        uint32_t ui32 = j;
        float floaty = static_cast<float>(j);
        float boaty = static_cast<float>(j);
        char coal = static_cast<char>(j);

        uint32_t ui32_out = 0;
        float floaty_out = 0;
        float boaty_out = 0;
        char coal_out = 0;

        ss.deserialize(ui32_out, skyloupe::Endianness::Native);
        ss.deserialize(floaty_out, skyloupe::Endianness::Native);
        ss.deserialize(boaty_out, skyloupe::Endianness::Native);
        ss.deserialize(coal_out, skyloupe::Endianness::Native);

        ASSERT_EQ(ui32, ui32_out);
        ASSERT_EQ(floaty, floaty_out);
        ASSERT_EQ(boaty, boaty_out);
        ASSERT_EQ(coal, coal_out);
    }

    ASSERT_EQ(ss.byte_size(), 0);
}

TEST_F(StreamUnitTest, OutofBoundRead)
{
    skyloupe::Stream ss(512, 32);
    const size_t NUMWRITES = 10;
    double boaty = 0.0;
    double* array_out = new double[NUMWRITES];

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        array_out[i] = 0.0;
        boaty = static_cast<double>(i);
        size_t bytes = ss.write(&boaty, sizeof(boaty));
        ASSERT_EQ(bytes, sizeof(boaty));
    }

    size_t bytes = NUMWRITES * sizeof(boaty);
    ASSERT_EQ(ss.byte_size(), bytes);

    bytes = ss.read(array_out, NUMWRITES * sizeof(double));
    ASSERT_EQ(bytes, NUMWRITES * sizeof(double));

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        ASSERT_EQ(array_out[i], static_cast<double>(i));
    }

    //Extra read, should return zero
    bytes = ss.read(array_out, sizeof(double));
    ASSERT_EQ(bytes, 0);

    delete[] array_out;
}

// Stream's PZStream binds to Stream's own buffer, so a copy would alias it.
static_assert(!std::is_copy_constructible<skyloupe::Stream>::value, "Stream must not be copyable");
static_assert(!std::is_copy_assignable<skyloupe::Stream>::value, "Stream must not be copy-assignable");

TEST_F(StreamUnitTest, ClearThenLargeWrite)
{
    // clear() used to leave the write position behind, so the next write
    // landed past the end of the shrunken buffer.
    skyloupe::Stream ss(16, 16);
    std::vector<char> first(64, 'a');
    std::vector<char> second(1000);
    std::vector<char> out(1000, 0);

    for (size_t i = 0; i < second.size(); i++)
    {
        second[i] = static_cast<char>(i);
    }

    ASSERT_EQ(ss.write(first.data(), first.size()), first.size());
    ss.clear();
    ASSERT_EQ(ss.byte_size(), 0);

    ASSERT_EQ(ss.write(second.data(), second.size()), second.size());
    ASSERT_EQ(ss.byte_size(), second.size());
    ASSERT_EQ(ss.read(out.data(), out.size()), out.size());
    ASSERT_EQ(out, second);
}

TEST_F(StreamUnitTest, PartialReadKeepsOrderAcrossGrowth)
{
    // Reads advance an offset; a later write that needs room compacts the
    // unread bytes to the front before growing.
    skyloupe::Stream ss(16, 8);
    uint32_t out = 0;

    for (uint32_t i = 0; i < 4; i++)
    {
        ss.serialize(i, skyloupe::Endianness::Big);
    }

    ss.deserialize(out, skyloupe::Endianness::Big);
    ASSERT_EQ(out, 0u);
    ss.deserialize(out, skyloupe::Endianness::Big);
    ASSERT_EQ(out, 1u);
    ASSERT_EQ(ss.byte_size(), 2 * sizeof(uint32_t));

    for (uint32_t i = 4; i < 64; i++)
    {
        ss.serialize(i, skyloupe::Endianness::Big);
    }

    ASSERT_EQ(ss.byte_size(), 62 * sizeof(uint32_t));
    for (uint32_t i = 2; i < 64; i++)
    {
        ss.deserialize(out, skyloupe::Endianness::Big);
        ASSERT_EQ(out, i);
    }
    ASSERT_EQ(ss.byte_size(), 0);
}

TEST_F(StreamUnitTest, DataPointsAtUnreadBytes)
{
    skyloupe::Stream ss;
    const char bytes[] = { 1, 2, 3, 4 };
    char out = 0;

    ss.write(bytes, sizeof(bytes));
    ss.read(&out, 1);

    ASSERT_EQ(ss.byte_size(), 3);
    ASSERT_EQ(0, std::memcmp(ss.data(), bytes + 1, 3));
}

TEST_F(StreamUnitTest, ManySmallReads)
{
    // Each read used to erase from the front of the vector, making this
    // quadratic in the message size.
    const size_t COUNT = 1 << 20;
    skyloupe::Stream ss;
    std::vector<unsigned char> in(COUNT);

    for (size_t i = 0; i < COUNT; i++)
    {
        in[i] = static_cast<unsigned char>(i * 31);
    }
    ss.write(in.data(), in.size());

    for (size_t i = 0; i < COUNT; i++)
    {
        unsigned char b = 0;
        ss.deserialize(b, skyloupe::Endianness::Native);
        ASSERT_EQ(b, in[i]);
    }
    ASSERT_EQ(ss.byte_size(), 0);
}

TEST_F(StreamUnitTest, ClearKeepsBuffer)
{
    // clear() keeps the buffer, so the next write neither reallocates nor
    // zero-fills it.
    skyloupe::Stream ss(64, 64);
    const char bytes[] = { 1, 2, 3, 4 };

    ss.write(bytes, sizeof(bytes));
    const char* before = ss.data();
    const size_t size = ss.size();
    ss.clear();
    ASSERT_EQ(ss.byte_size(), 0u);
    ASSERT_EQ(ss.size(), size);

    ss.write(bytes, sizeof(bytes));
    ASSERT_EQ(ss.data(), before);
    ASSERT_EQ(ss.byte_size(), sizeof(bytes));
}

TEST_F(StreamUnitTest, ClaimAndConsume)
{
    skyloupe::Stream ss(8, 8);
    const char bytes[] = { 10, 20, 30, 40, 50, 60 };

    // A claim counts as written, growing the buffer when needed.
    char* p = ss.claim(4);
    ASSERT_NE(p, nullptr);
    std::memcpy(p, bytes, 4);
    p = ss.claim(sizeof(bytes) + 16);
    ASSERT_NE(p, nullptr);
    std::memset(p, 7, sizeof(bytes) + 16);
    ASSERT_EQ(ss.byte_size(), 4 + sizeof(bytes) + 16);

    // A short consume takes nothing.
    ASSERT_EQ(ss.consume(ss.byte_size() + 1), nullptr);
    ASSERT_EQ(ss.byte_size(), 4 + sizeof(bytes) + 16);

    const char* q = ss.consume(4);
    ASSERT_NE(q, nullptr);
    ASSERT_EQ(0, std::memcmp(q, bytes, 4));
    q = ss.consume(sizeof(bytes) + 16);
    ASSERT_NE(q, nullptr);
    ASSERT_EQ(q[0], 7);
    ASSERT_EQ(ss.byte_size(), 0u);

    ASSERT_EQ(ss.claim(0), nullptr);
    ASSERT_EQ(ss.consume(0), nullptr);
}

TEST_F(StreamUnitTest, ClaimReusesReadSpace)
{
    // A claim that does not fit slides unread bytes to the front first.
    skyloupe::Stream ss(8, 8);
    const char bytes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    char out[6] = {};

    ss.write(bytes, sizeof(bytes));
    ss.read(out, sizeof(out));
    char* p = ss.claim(4);
    ASSERT_NE(p, nullptr);
    std::memcpy(p, bytes, 4);

    ASSERT_EQ(ss.size(), 8u);
    ASSERT_EQ(ss.byte_size(), 6u);
    const char want[] = { 7, 8, 1, 2, 3, 4 };
    ASSERT_EQ(0, std::memcmp(ss.data(), want, sizeof(want)));
}

TEST_F(StreamUnitTest, ClaimAndConsumeNeedSwMode)
{
    skyloupe::Stream ss(512, 512, skyloupe::Stream::PZ);
    ASSERT_EQ(ss.claim(4), nullptr);
    ASSERT_EQ(ss.consume(0), nullptr);
}

TEST_F(StreamUnitTest, WireByteOrder)
{
    char buf[8];
    const char* end = nullptr;

    // Big and little endian are fixed on the wire; native is the host's order.
    ASSERT_EQ(skyloupe::wire::put(buf, uint32_t(0x01020304), skyloupe::Endianness::Big), buf + 4);
    ASSERT_EQ(0, std::memcmp(buf, "\x01\x02\x03\x04", 4));
    skyloupe::wire::put(buf, uint32_t(0x01020304), skyloupe::Endianness::Little);
    ASSERT_EQ(0, std::memcmp(buf, "\x04\x03\x02\x01", 4));
    const uint32_t host = 0x01020304;
    skyloupe::wire::put(buf, host, skyloupe::Endianness::Native);
    ASSERT_EQ(0, std::memcmp(buf, &host, 4));

    skyloupe::wire::put(buf, int16_t(-2), skyloupe::Endianness::Big);
    ASSERT_EQ(0, std::memcmp(buf, "\xff\xfe", 2));

    // 3.5f is 0x40600000; 3.5 is 0x400c000000000000.
    skyloupe::wire::put(buf, 3.5f, skyloupe::Endianness::Big);
    ASSERT_EQ(0, std::memcmp(buf, "\x40\x60\x00\x00", 4));
    skyloupe::wire::put(buf, 3.5, skyloupe::Endianness::Little);
    ASSERT_EQ(0, std::memcmp(buf, "\x00\x00\x00\x00\x00\x00\x0c\x40", 8));

    double d = 0;
    end = skyloupe::wire::get(buf, d, skyloupe::Endianness::Little);
    ASSERT_EQ(end, buf + 8);
    ASSERT_EQ(d, 3.5);

    uint64_t u = 0;
    std::memcpy(buf, "\x00\x00\x00\x02\xcb\x41\x78\x00", 8);
    skyloupe::wire::get(buf, u, skyloupe::Endianness::Big);
    ASSERT_EQ(u, 12000000000ull);

    char text[3] = {};
    ASSERT_EQ(skyloupe::wire::put_bytes(buf, "sky", 3), buf + 3);
    ASSERT_EQ(skyloupe::wire::get_bytes(buf, text, 3), buf + 3);
    ASSERT_EQ(0, std::memcmp(text, "sky", 3));
}
