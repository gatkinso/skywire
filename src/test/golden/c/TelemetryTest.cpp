// This is the generated .c file

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstring>
#include "Telemetry.h"

TEST(TelemetryTest, SerDes)
{
    Cstream cs;

    const int NUM = 1000;
    Telemetry out[NUM];
    cstream_init(&cs, 128);
    int sign = 1;
    for (uint32_t i = 0; i < NUM; i++)
    {
        out[i].device_id = i*7;
        out[i].timestamp_us = i*18;
        out[i].mode = 5;
        out[i].status = 5;
        out[i].armed = 1;
        strncpy(out[i].callsign, "skywire", 8);
        memset(&out[i].position, 0, sizeof(out[i].position));
        out[i].battery_mv = i*15;
        sign *= -1;
        Telemetry_Serialize(&out[i], &cs);
    }

    for (uint32_t i = 0; i < NUM; i++)
    {
        Telemetry in;
        Telemetry_Deserialize(&in, &cs);

        ASSERT_EQ(1, Telemetry_Equals(&out[i], &in));
    }

    cstream_fini(&cs);
}

TEST(TelemetryTest, Size)
{
    ASSERT_EQ(Telemetry_Size(),sizeof(uint32_t)+sizeof(uint64_t)+sizeof(uint8_t)+8+Position_Size()+sizeof(uint16_t));
}

TEST(TelemetryTest, ShortBuffer)
{
    Telemetry in;
    std::memset(&in, 0x5a, sizeof(in));
    Cstream full;
    cstream_init(&full, 128);
    ASSERT_EQ(0, Telemetry_Serialize(&in, &full));
    ASSERT_EQ(Telemetry_Size(), cstream_available(&full));

    Cstream cs;
    cstream_init(&cs, 128);
    cstream_write(&cs, full.head_, Telemetry_Size() - 1, SW_NATIVE_ENDIAN_FLAG);

    Telemetry out;
    std::memset(&out, 0x11, sizeof(out));
    Telemetry before = out;

    ASSERT_EQ(-1, Telemetry_Deserialize(&out, &cs));
    ASSERT_EQ(0, std::memcmp(&out, &before, sizeof(out)));
    ASSERT_EQ(Telemetry_Size() - 1, cstream_available(&cs));

    cstream_fini(&cs);
    cstream_fini(&full);
}

TEST(TelemetryTest, BitfieldSetters)
{
    Telemetry msg;
    std::memset(&msg, 0, sizeof(msg));
    {
        uint8_t value = (uint8_t)0x15ull, got = 0;
        ASSERT_EQ(0, Set_Telemetry_mode(&msg, &value));
        ASSERT_EQ(0, Get_Telemetry_mode(&msg, &got));
        ASSERT_EQ((uint8_t)5, got);
    }
    {
        uint8_t value = (uint8_t)0xdull, got = 0;
        ASSERT_EQ(0, Set_Telemetry_status(&msg, &value));
        ASSERT_EQ(0, Get_Telemetry_status(&msg, &got));
        ASSERT_EQ((uint8_t)5, got);
    }
    {
        uint8_t value = (uint8_t)0x3ull, got = 0;
        ASSERT_EQ(0, Set_Telemetry_armed(&msg, &value));
        ASSERT_EQ(0, Get_Telemetry_armed(&msg, &got));
        ASSERT_EQ((uint8_t)1, got);
    }
}

