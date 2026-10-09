#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "stream.h"

class PZStreamUnitTest : public ::testing::Test
{
protected:
    PZStreamUnitTest() {}
};

TEST_F(PZStreamUnitTest, DISABLED_Serialize)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);

    ss.serialize(ui32, skyloupe::Endianness::Native);
    ss.serialize(floaty, skyloupe::Endianness::Native);

    ASSERT_EQ(ss.byte_size(), sizeof(ui32) /*+ sizeof(floaty)+1*/);
}

TEST_F(PZStreamUnitTest, SerDes)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);
    float boaty = static_cast<float>(654.321);
    //char coal = 'G';

    uint32_t ui32_out = 0;
    float floaty_out = static_cast<float>(0);
    float boaty_out = static_cast<float>(0);
    //char coal_out = 0;

    ss.serialize(ui32, skyloupe::Endianness::Native);
    //ss.serialize(floaty, skyloupe::Endianness::Native);
    //ss.serialize(boaty, skyloupe::Endianness::Native);
    //ss.serialize(coal, skyloupe::Endianness::Native);

    //ASSERT_EQ(ss.byte_size(), sizeof(ui32) + sizeof(floaty) + sizeof(boaty) + sizeof(coal));

    ss.deserialize(ui32_out, skyloupe::Endianness::Native);
    //ss.deserialize(floaty_out, skyloupe::Endianness::Native);
    //ss.deserialize(boaty_out, skyloupe::Endianness::Native);
    //ss.deserialize(coal_out, skyloupe::Endianness::Native);

    //ASSERT_EQ(ss.byte_size(), 0);

    ASSERT_EQ(ui32, ui32_out);
    //ASSERT_EQ(floaty, floaty_out);
    //ASSERT_EQ(boaty, boaty_out);
    //ASSERT_EQ(coal, coal_out);
}

TEST_F(PZStreamUnitTest, WriteBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss(512, 512, skyloupe::Stream::PZ);
    uint64_t* big_array = new uint64_t[ARRAYSIZE];

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
    }

    size_t bytes = ss.write(big_array, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    bytes = ARRAYSIZE * sizeof(uint64_t);
    //ASSERT_EQ(ss.byte_size(), bytes);
    
    delete [] big_array;
}

TEST_F(PZStreamUnitTest, WriteReadBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss(512, 512, skyloupe::Stream::PZ);
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
    //ASSERT_EQ(ss.byte_size(), bytes);

    bytes = ss.read(big_array_out, ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        ASSERT_EQ(big_array_out[i], i);
    }

    delete [] big_array_out;
    delete [] big_array;
}

TEST_F(PZStreamUnitTest, WriteLittleGrows)
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

TEST_F(PZStreamUnitTest, WriteReadLittleGrows)
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

TEST_F(PZStreamUnitTest, SequentialSerDes)
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

TEST_F(PZStreamUnitTest, InterleavedSerDes)
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

TEST_F(PZStreamUnitTest, OutofBoundRead)
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
