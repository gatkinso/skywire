#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "stream.h"

// In PZ mode each value is one protobuf field, numbered from 1 in call order.
// Values are read back after prepare(), which points the reader at the start
// of the message. byte_size() is the size of the whole message.

class PZStreamUnitTest : public ::testing::Test
{
protected:
    PZStreamUnitTest() {}

    static std::vector<unsigned char> bytes_of(const skyloupe::Stream& ss)
    {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(ss.data());
        return std::vector<unsigned char>(p, p + ss.size());
    }
};

TEST_F(PZStreamUnitTest, Serialize)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    uint32_t ui32 = 0xCAFEBABE;
    float floaty = static_cast<float>(123.456);

    ss.serialize(ui32, skyloupe::Endianness::Native);
    ss.serialize(floaty, skyloupe::Endianness::Native);

    // Field 1 varint 0xCAFEBABE, field 2 fixed32 123.456f (encoded independently).
    const std::vector<unsigned char> expected = {
        0x08, 0xbe, 0xf5, 0xfa, 0xd7, 0x0c,
        0x15, 0x79, 0xe9, 0xf6, 0x42 };

    ASSERT_EQ(ss.byte_size(), expected.size());
    ASSERT_EQ(bytes_of(ss), expected);
}

TEST_F(PZStreamUnitTest, NegativeIntIsTenByteVarint)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    int32_t minus_one = -1;

    ss.serialize(minus_one, skyloupe::Endianness::Native);

    // Protobuf int32 sign-extends negatives to 64 bits.
    const std::vector<unsigned char> expected = {
        0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01 };

    ASSERT_EQ(bytes_of(ss), expected);
}

TEST_F(PZStreamUnitTest, SerDes)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
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

    ss.prepare();

    ASSERT_TRUE(ss.deserialize(ui32_out, skyloupe::Endianness::Native));
    ASSERT_TRUE(ss.deserialize(floaty_out, skyloupe::Endianness::Native));
    ASSERT_TRUE(ss.deserialize(boaty_out, skyloupe::Endianness::Native));
    ASSERT_TRUE(ss.deserialize(coal_out, skyloupe::Endianness::Native));

    ASSERT_EQ(ui32, ui32_out);
    ASSERT_EQ(floaty, floaty_out);
    ASSERT_EQ(boaty, boaty_out);
    ASSERT_EQ(coal, coal_out);
}

TEST_F(PZStreamUnitTest, SerDesEveryTypeAtItsLimits)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);

    const char c_min = std::numeric_limits<char>::min();
    const signed char sc_min = std::numeric_limits<signed char>::min();
    const unsigned char uc_max = std::numeric_limits<unsigned char>::max();
    const int16_t i16_min = std::numeric_limits<int16_t>::min();
    const uint16_t u16_max = std::numeric_limits<uint16_t>::max();
    const int32_t i32_min = std::numeric_limits<int32_t>::min();
    const uint32_t u32_max = std::numeric_limits<uint32_t>::max();
    const int64_t i64_min = std::numeric_limits<int64_t>::min();
    const uint64_t u64_max = std::numeric_limits<uint64_t>::max();
    const float f_low = std::numeric_limits<float>::lowest();
    const double d_max = std::numeric_limits<double>::max();

    ss.serialize(c_min);
    ss.serialize(sc_min);
    ss.serialize(uc_max);
    ss.serialize(i16_min);
    ss.serialize(u16_max);
    ss.serialize(i32_min);
    ss.serialize(u32_max);
    ss.serialize(i64_min);
    ss.serialize(u64_max);
    ss.serialize(f_low);
    ss.serialize(d_max);

    ss.prepare();

    char c = 0;
    signed char sc = 0;
    unsigned char uc = 0;
    int16_t i16 = 0;
    uint16_t u16 = 0;
    int32_t i32 = 0;
    uint32_t u32 = 0;
    int64_t i64 = 0;
    uint64_t u64 = 0;
    float f = 0;
    double d = 0;

    ASSERT_TRUE(ss.deserialize(c));
    ASSERT_TRUE(ss.deserialize(sc));
    ASSERT_TRUE(ss.deserialize(uc));
    ASSERT_TRUE(ss.deserialize(i16));
    ASSERT_TRUE(ss.deserialize(u16));
    ASSERT_TRUE(ss.deserialize(i32));
    ASSERT_TRUE(ss.deserialize(u32));
    ASSERT_TRUE(ss.deserialize(i64));
    ASSERT_TRUE(ss.deserialize(u64));
    ASSERT_TRUE(ss.deserialize(f));
    ASSERT_TRUE(ss.deserialize(d));

    ASSERT_EQ(c, c_min);
    ASSERT_EQ(sc, sc_min);
    ASSERT_EQ(uc, uc_max);
    ASSERT_EQ(i16, i16_min);
    ASSERT_EQ(u16, u16_max);
    ASSERT_EQ(i32, i32_min);
    ASSERT_EQ(u32, u32_max);
    ASSERT_EQ(i64, i64_min);
    ASSERT_EQ(u64, u64_max);
    ASSERT_EQ(f, f_low);
    ASSERT_EQ(d, d_max);

    // Every field consumed: the next read fails and leaves its target alone.
    uint32_t extra = 7;
    ASSERT_FALSE(ss.deserialize(extra));
    ASSERT_EQ(extra, 7u);
}

TEST_F(PZStreamUnitTest, DeserializeBeforePrepareFindsNothing)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    uint32_t ui32 = 0xCAFEBABE;
    uint32_t ui32_out = 0;

    ss.serialize(ui32);

    ASSERT_FALSE(ss.deserialize(ui32_out));
    ASSERT_EQ(ui32_out, 0u);

    ss.prepare();
    ASSERT_TRUE(ss.deserialize(ui32_out));
    ASSERT_EQ(ui32_out, ui32);
}

TEST_F(PZStreamUnitTest, PrepareRewinds)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    int64_t value = -9000000000;
    int64_t out = 0;

    ss.serialize(value);

    for (int pass = 0; pass < 2; pass++)
    {
        out = 0;
        ss.prepare();
        ASSERT_TRUE(ss.deserialize(out));
        ASSERT_EQ(out, value);
        ASSERT_FALSE(ss.deserialize(out));
    }
}

TEST_F(PZStreamUnitTest, WriteBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss(512, 512, skyloupe::Stream::PZ);
    std::vector<uint64_t> big_array(ARRAYSIZE);

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
    }

    size_t bytes = ss.write(big_array.data(), ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    // One bytes field: key 0x0a, a 3-byte varint length, then the data.
    ASSERT_EQ(ss.byte_size(), 1 + 3 + bytes);
}

TEST_F(PZStreamUnitTest, WriteReadBigGrow)
{
    const size_t ARRAYSIZE = 100000;
    skyloupe::Stream ss(512, 512, skyloupe::Stream::PZ);
    std::vector<uint64_t> big_array(ARRAYSIZE);
    std::vector<uint64_t> big_array_out(ARRAYSIZE, 0);

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        big_array[i] = i;
    }

    size_t bytes = ss.write(big_array.data(), ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    ss.prepare();

    bytes = ss.read(big_array_out.data(), ARRAYSIZE * sizeof(uint64_t));
    ASSERT_EQ(bytes, ARRAYSIZE * sizeof(uint64_t));

    for (uint64_t i = 0; i < static_cast<uint64_t>(ARRAYSIZE); i++)
    {
        ASSERT_EQ(big_array_out[i], i);
    }
}

TEST_F(PZStreamUnitTest, WriteReadManyFields)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    // Enough values to run past the field numbers protobuf reserves.
    const size_t NUMWRITES = 20000;

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        ss.serialize(static_cast<double>(i));
    }

    ss.prepare();

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        double out = -1.0;
        ASSERT_TRUE(ss.deserialize(out));
        ASSERT_EQ(out, static_cast<double>(i));
    }

    double out = -1.0;
    ASSERT_FALSE(ss.deserialize(out));
}

TEST_F(PZStreamUnitTest, FieldNumbersSkipReservedRange)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    const uint32_t NUMWRITES = 19001;

    for (uint32_t i = 0; i < NUMWRITES; i++)
    {
        ss.serialize(i);
    }

    // Read the field numbers with a plain protozero reader.
    protozero::pbf_reader reader(ss.data(), ss.size());
    std::vector<uint32_t> tags;
    while (reader.next())
    {
        tags.push_back(reader.tag());
        reader.skip();
    }

    ASSERT_EQ(tags.size(), NUMWRITES);
    ASSERT_EQ(tags[0], 1u);
    ASSERT_EQ(tags[18997], 18998u);
    ASSERT_EQ(tags[18998], 18999u);
    ASSERT_EQ(tags[18999], 20000u);
    ASSERT_EQ(tags[19000], 20001u);
}

// Exposes the field counter, which no real message can drive to 2^29.
class PZStreamAtLimit : public skyloupe::PZStream
{
public:
    explicit PZStreamAtLimit(std::string& buffer)
    :PZStream(buffer)
    {
        current_tag_ = max_field_number;
    }
};

TEST_F(PZStreamUnitTest, TooManyFieldsThrows)
{
    std::string buffer;
    PZStreamAtLimit pz(buffer);
    uint32_t value = 1;

    // The largest field number is still usable...
    ASSERT_NO_THROW(pz.serialize(value, skyloupe::Endianness::Native));
    const size_t size = buffer.size();

    // ...the next one does not exist, and nothing is written.
    ASSERT_THROW(pz.serialize(value, skyloupe::Endianness::Native), std::out_of_range);
    ASSERT_THROW(pz.write(&value, sizeof(value)), std::out_of_range);
    ASSERT_EQ(buffer.size(), size);
}

TEST_F(PZStreamUnitTest, ReadWrongLengthFails)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    const char text[] = "skywire";
    char out[16];
    std::memset(out, 0x5a, sizeof(out));

    ASSERT_EQ(ss.write(text, 7), 7u);
    ss.prepare();

    ASSERT_EQ(ss.read(out, 6), 0u);
    ASSERT_EQ(out[0], 0x5a);
}

TEST_F(PZStreamUnitTest, WriteZeroBytesWritesNothing)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    char c = 0;

    ASSERT_EQ(ss.write(&c, 0), 0u);
    ASSERT_EQ(ss.byte_size(), 0u);
}

TEST_F(PZStreamUnitTest, OutofBoundRead)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    const size_t NUMWRITES = 10;
    std::vector<double> array_out(NUMWRITES, 0.0);

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        double boaty = static_cast<double>(i);
        size_t bytes = ss.write(&boaty, sizeof(boaty));
        ASSERT_EQ(bytes, sizeof(boaty));
    }

    ss.prepare();

    for (size_t i = 0; i < NUMWRITES; i++)
    {
        size_t bytes = ss.read(&array_out[i], sizeof(double));
        ASSERT_EQ(bytes, sizeof(double));
        ASSERT_EQ(array_out[i], static_cast<double>(i));
    }

    //Extra read, should return zero
    double extra = 0.0;
    ASSERT_EQ(ss.read(&extra, sizeof(double)), 0u);
}

TEST_F(PZStreamUnitTest, ClearAndResetTagStartsNewMessage)
{
    skyloupe::Stream ss(0, 0, skyloupe::Stream::PZ);
    skyloupe::Stream fresh(0, 0, skyloupe::Stream::PZ);
    uint32_t first = 1;
    uint32_t second = 0xCAFEBABE;

    ss.serialize(first);
    ss.serialize(first);
    ss.clear();
    ss.reset_tag();
    ss.serialize(second);

    fresh.serialize(second);

    ASSERT_EQ(bytes_of(ss), bytes_of(fresh));

    uint32_t out = 0;
    ss.prepare();
    ASSERT_TRUE(ss.deserialize(out));
    ASSERT_EQ(out, second);
    ASSERT_FALSE(ss.deserialize(out));
}
