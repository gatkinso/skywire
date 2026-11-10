// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>
#include "Position.h"

namespace telemetry {
    class Telemetry : public skyloupe::Streamable {
    public:
        Telemetry() {}
        virtual ~Telemetry() {}
        bool serialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) const {
            if (str.mode() == skyloupe::Stream::PZ) {
                protozero::pbf_writer pbf{str.pz_buffer()};
                return serialize_pb(pbf);
            }
            char* p = str.claim(size());
            if (p == nullptr) return false;
            serialize_sw(p);
            return true;
        }
        // Writes the SW wire form, size() bytes, at p; returns the end.
        char* serialize_sw(char* p) const {
            p = skyloupe::wire::put(p, device_id, skyloupe::Endianness::Big);
            p = skyloupe::wire::put(p, timestamp_us, skyloupe::Endianness::Little);
            uint8_t unit_0 = 0;
            unit_0 |= static_cast<uint8_t>((static_cast<uint8_t>(mode) & 0xfu) << 0);
            unit_0 |= static_cast<uint8_t>((static_cast<uint8_t>(status) & 0x7u) << 4);
            unit_0 |= static_cast<uint8_t>((static_cast<uint8_t>(armed) & 0x1u) << 7);
            p = skyloupe::wire::put(p, unit_0, skyloupe::Endianness::Big);
            p = skyloupe::wire::put_bytes(p, callsign, 8);
            p = position.serialize_sw(p);
            p = skyloupe::wire::put(p, battery_mv, skyloupe::Endianness::Little);
            return p;
        }
        bool deserialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) {
            if (str.mode() == skyloupe::Stream::PZ) {
                Telemetry decoded;
                try {
                    if (false == decoded.deserialize_pb(protozero::pbf_reader{str.pz_buffer()})) return false;
                } catch (const protozero::exception&) {
                    return false;
                }
                *this = decoded;
                str.clear();
                return true;
            }
            const char* p = str.consume(size());
            if (p == nullptr) return false;
            deserialize_sw(p);
            return true;
        }
        // Reads the SW wire form, size() bytes, from p; returns the end.
        const char* deserialize_sw(const char* p) {
            p = skyloupe::wire::get(p, device_id, skyloupe::Endianness::Big);
            p = skyloupe::wire::get(p, timestamp_us, skyloupe::Endianness::Little);
            uint8_t unit_0 = 0;
            p = skyloupe::wire::get(p, unit_0, skyloupe::Endianness::Big);
            mode = (uint8_t)((uint8_t)(unit_0 >> 0) & 0xfu);
            status = (uint8_t)((uint8_t)(unit_0 >> 4) & 0x7u);
            armed = (uint8_t)((uint8_t)(unit_0 >> 7) & 0x1u);
            p = skyloupe::wire::get_bytes(p, callsign, 8);
            p = position.deserialize_sw(p);
            p = skyloupe::wire::get(p, battery_mv, skyloupe::Endianness::Little);
            return p;
        }
        bool serialize_pb(protozero::pbf_writer& pbf) const {
            if (skyloupe::pb_nonzero(device_id)) pbf.add_uint32(1, device_id);
            if (skyloupe::pb_nonzero(timestamp_us)) pbf.add_uint64(2, timestamp_us);
            if (skyloupe::pb_nonzero(mode)) pbf.add_uint32(3, mode);
            if (skyloupe::pb_nonzero(status)) pbf.add_uint32(4, status);
            if (skyloupe::pb_nonzero(armed)) pbf.add_uint32(5, armed);
            if (size_t len = skyloupe::pb_bytes_length(callsign, 8)) pbf.add_bytes(6, callsign, len);
            {
                protozero::pbf_writer sub{pbf, 7};
                if (false == position.serialize_pb(sub)) return false;
            }
            if (skyloupe::pb_nonzero(battery_mv)) pbf.add_uint32(8, battery_mv);
            return true;
        }
        bool deserialize_pb(protozero::pbf_reader pbf) {
            while (pbf.next()) {
                switch (pbf.tag_and_type()) {
                case protozero::tag_and_type(1, protozero::pbf_wire_type::varint): device_id = static_cast<uint32_t>(pbf.get_uint32()); break;
                case protozero::tag_and_type(2, protozero::pbf_wire_type::varint): timestamp_us = static_cast<uint64_t>(pbf.get_uint64()); break;
                case protozero::tag_and_type(3, protozero::pbf_wire_type::varint): mode = (uint8_t)((uint8_t)(pbf.get_uint32()) & 0xfu); break;
                case protozero::tag_and_type(4, protozero::pbf_wire_type::varint): status = (uint8_t)((uint8_t)(pbf.get_uint32()) & 0x7u); break;
                case protozero::tag_and_type(5, protozero::pbf_wire_type::varint): armed = (uint8_t)((uint8_t)(pbf.get_uint32()) & 0x1u); break;
                case protozero::tag_and_type(6, protozero::pbf_wire_type::length_delimited): if (false == skyloupe::pb_read_bytes(pbf, callsign, 8)) return false; break;
                case protozero::tag_and_type(7, protozero::pbf_wire_type::length_delimited): if (false == position.deserialize_pb(pbf.get_message())) return false; break;
                case protozero::tag_and_type(8, protozero::pbf_wire_type::varint): battery_mv = static_cast<uint16_t>(pbf.get_uint32()); break;
                default: pbf.skip(); break;
                }
            }
            return true;
        }
        static size_t size() {
            return(
                sizeof(uint32_t) +
                sizeof(uint64_t) +
                sizeof(uint8_t) +
                8 +
                Position::size() +
                sizeof(uint16_t)
            );
        }
        bool operator==(const Telemetry& other) const {
            if(this == &other) return true;
            return(
                device_id == other.device_id &&
                timestamp_us == other.timestamp_us &&
                mode == other.mode &&
                status == other.status &&
                armed == other.armed &&
                memcmp(callsign, other.callsign, 8) == 0 &&
                position == other.position &&
                battery_mv == other.battery_mv
            );
        }
        uint32_t Get_device_id() const { return device_id; }
        uint64_t Get_timestamp_us() const { return timestamp_us; }
        uint8_t Get_mode() const { return mode; }
        uint8_t Get_status() const { return status; }
        uint8_t Get_armed() const { return armed; }
        const char* Get_callsign() const { return callsign; } // 8 bytes; not NUL-terminated when full
        Position Get_position() const { return position; }
        uint16_t Get_battery_mv() const { return battery_mv; }
        void Set_device_id(const uint32_t newval) { device_id = newval; }
        void Set_timestamp_us(const uint64_t newval) { timestamp_us = newval; }
        void Set_mode(const uint8_t newval) { mode = (uint8_t)((uint8_t)(newval) & 0xfu); }
        void Set_status(const uint8_t newval) { status = (uint8_t)((uint8_t)(newval) & 0x7u); }
        void Set_armed(const uint8_t newval) { armed = (uint8_t)((uint8_t)(newval) & 0x1u); }
        void Set_callsign(const char* newval) { strncpy(callsign, newval, 8); }
        void Set_position(const Position newval) { position = newval; }
        void Set_battery_mv(const uint16_t newval) { battery_mv = newval; }
    private:
        uint32_t device_id{};
        uint64_t timestamp_us{};
        uint8_t mode{};
        uint8_t status{};
        uint8_t armed{};
        char callsign[8]{};
        Position position{};
        uint16_t battery_mv{};
    };
}
