// This is the generated file

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include "Telemetry.h"

TEST(TelemetryTest, SerDes)
{
    telemetry::Telemetry in, out;
    skyloupe::Stream stream;

    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);
}

TEST(TelemetryTest, Size)
{
    telemetry::Telemetry msg;

    ASSERT_EQ(msg.size(), sizeof(uint32_t)+sizeof(uint64_t)+sizeof(uint8_t)+8+telemetry::Position::size()+sizeof(uint16_t));
}

TEST(TelemetryTest, EqualityOperator)
{
    telemetry::Telemetry in, out;
    skyloupe::Stream stream;

    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);

    ASSERT_EQ(in, out);
}

TEST(TelemetryTest, ShortBuffer)
{
    telemetry::Telemetry in;
    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    skyloupe::Stream full;
    ASSERT_TRUE(in.serialize(full));
    ASSERT_EQ(full.byte_size(), in.size());

    skyloupe::Stream stream;
    stream.write(full.data(), full.byte_size() - 1);

    telemetry::Telemetry out = in;
    ASSERT_FALSE(out.deserialize(stream));
    ASSERT_EQ(out, in);
    ASSERT_EQ(stream.byte_size(), in.size() - 1);
}

TEST(TelemetryTest, BitfieldSetters)
{
    telemetry::Telemetry msg;
    msg.Set_mode(static_cast<uint8_t>(0x15ull));
    ASSERT_EQ(static_cast<uint8_t>(5), msg.Get_mode());
    msg.Set_status(static_cast<uint8_t>(0xdull));
    ASSERT_EQ(static_cast<uint8_t>(5), msg.Get_status());
    msg.Set_armed(static_cast<uint8_t>(0x3ull));
    ASSERT_EQ(static_cast<uint8_t>(1), msg.Get_armed());
}

TEST(TelemetryTest, PZRoundTrip)
{
    telemetry::Telemetry in, out;
    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(in, out);
    ASSERT_EQ(0u, stream.byte_size());
}

TEST(TelemetryTest, PZWire)
{
    telemetry::Telemetry in;
    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    static const uint8_t want[] = {
        0x08, 0x80, 0xbc, 0xc1, 0x96, 0x0b, 0x10, 0x80,
        0xf0, 0x85, 0xda, 0x2c, 0x18, 0x05, 0x20, 0x05,
        0x28, 0x01, 0x32, 0x07, 0x73, 0x6b, 0x79, 0x77,
        0x69, 0x72, 0x65, 0x40, 0xe0, 0xd4, 0x03,
    };

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(stream));
    ASSERT_EQ(sizeof(want), stream.byte_size());
    ASSERT_EQ(0, std::memcmp(stream.data(), want, sizeof(want)));
}

TEST(TelemetryTest, PZEmptyIsDefault)
{
    telemetry::Telemetry zero, in, out;
    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(zero.serialize(stream));
    ASSERT_EQ(0u, stream.byte_size());

    out = in;
    ASSERT_TRUE(out.deserialize(stream));
    ASSERT_EQ(zero, out);
}

TEST(TelemetryTest, PZShortBuffer)
{
    telemetry::Telemetry in;
    in.Set_device_id(3000000000);
    in.Set_timestamp_us(12000000000);
    in.Set_mode(5);
    in.Set_status(5);
    in.Set_armed(1);
    in.Set_callsign("skywire");
    in.Set_battery_mv(60000);

    skyloupe::Stream full(512, 512, skyloupe::Stream::PZ);
    ASSERT_TRUE(in.serialize(full));
    ASSERT_LT(0u, full.byte_size());

    skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);
    stream.pz_buffer() = full.pz_buffer().substr(0, full.byte_size() - 1);

    telemetry::Telemetry out;
    ASSERT_FALSE(out.deserialize(stream));
    ASSERT_EQ(out, telemetry::Telemetry());
    ASSERT_EQ(stream.byte_size(), full.byte_size() - 1);
}

