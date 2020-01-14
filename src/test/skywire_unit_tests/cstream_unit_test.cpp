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
