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

    ss.serialize(ui32);
    ss.serialize(floaty);

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

    ss.serialize(ui32);
    ss.serialize(floaty);
    ss.serialize(boaty);
    ss.serialize(coal);

    ASSERT_EQ(ss.byte_size(), sizeof(ui32) + sizeof(floaty) + sizeof(boaty) + sizeof(coal));

    ss.deserialize(ui32_out);
    ss.deserialize(floaty_out);
    ss.deserialize(boaty_out);
    ss.deserialize(coal_out);

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
    uint64_t big_array[ARRAYSIZE];

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
    }

    size_t bytes = ss.write(big_array, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    bytes = ARRAYSIZE * sizeof(uint64_t);
    ASSERT_EQ(ss.byte_size(), bytes);
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

    delete big_array_out;
    delete big_array;
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

    delete array_out;
}

TEST_F(StreamUnitTest, InterleavedSerDes)
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

            ss.serialize(ui32);
            ss.serialize(floaty);
            ss.serialize(boaty);
            ss.serialize(coal);

            ss.deserialize(ui32_out);
            ss.deserialize(floaty_out);
            ss.deserialize(boaty_out);
            ss.deserialize(coal_out);

            ASSERT_EQ(ss.byte_size(), 0);
            ASSERT_EQ(ui32, ui32_out);
            ASSERT_EQ(floaty, floaty_out);
            ASSERT_EQ(boaty, boaty_out);
            ASSERT_EQ(coal, coal_out);
        }
    }
}

