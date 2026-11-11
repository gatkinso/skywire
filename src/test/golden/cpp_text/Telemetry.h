// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>
#include "text.h"
#include "bson.h"
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
        std::string to_json() const {
            skyloupe::text::JsonWriter w;
            to_json(w);
            return w.take();
        }
        void to_json(skyloupe::text::JsonWriter& w) const {
            w.begin_object();
            w.key("device_id"); w.value(this->device_id);
            w.key("timestamp_us"); w.value(this->timestamp_us);
            w.key("mode"); w.value(this->mode);
            w.key("status"); w.value(this->status);
            w.key("armed"); w.value(this->armed);
            w.key("callsign"); w.bytes(this->callsign, 8);
            w.key("position"); this->position.to_json(w);
            w.key("battery_mv"); w.value(this->battery_mv);
            w.end_object();
        }
        bool from_json(std::string_view text) {
            skyloupe::text::JsonValue doc;
            if (false == skyloupe::text::parse_json(text, doc)) return false;
            return from_json(doc);
        }
        bool from_json(const skyloupe::text::JsonValue& value) {
            if (value.kind != skyloupe::text::JsonValue::Object) return false;
            Telemetry decoded;
            for (const auto& field : value.members) {
                if (field.name == "device_id") { if (false == skyloupe::text::read(field.value, decoded.device_id)) return false; continue; }
                if (field.name == "timestamp_us") { if (false == skyloupe::text::read(field.value, decoded.timestamp_us)) return false; continue; }
                if (field.name == "mode") { if (false == skyloupe::text::read(field.value, decoded.mode, 4)) return false; continue; }
                if (field.name == "status") { if (false == skyloupe::text::read(field.value, decoded.status, 3)) return false; continue; }
                if (field.name == "armed") { if (false == skyloupe::text::read(field.value, decoded.armed, 1)) return false; continue; }
                if (field.name == "callsign") { if (false == skyloupe::text::read_bytes(field.value, decoded.callsign, 8)) return false; continue; }
                if (field.name == "position") { if (false == decoded.position.from_json(field.value)) return false; continue; }
                if (field.name == "battery_mv") { if (false == skyloupe::text::read(field.value, decoded.battery_mv)) return false; continue; }
            }
            *this = decoded;
            return true;
        }
        std::string to_xml() const {
            skyloupe::text::XmlWriter w;
            to_xml(w, "Telemetry");
            return w.take();
        }
        void to_xml(skyloupe::text::XmlWriter& w, std::string_view element) const {
            w.open(element);
            w.element("device_id", this->device_id);
            w.element("timestamp_us", this->timestamp_us);
            w.element("mode", this->mode);
            w.element("status", this->status);
            w.element("armed", this->armed);
            w.bytes_element("callsign", this->callsign, 8);
            this->position.to_xml(w, "position");
            w.element("battery_mv", this->battery_mv);
            w.close(element);
        }
        bool from_xml(std::string_view text) {
            skyloupe::text::XmlNode doc;
            if (false == skyloupe::text::parse_xml(text, doc) || doc.name != "Telemetry") return false;
            return from_xml(doc);
        }
        bool from_xml(const skyloupe::text::XmlNode& node) {
            if (false == skyloupe::text::is_blank(node)) return false;
            Telemetry decoded;
            for (const auto& child : node.children) {
                if (child.name == "device_id") { if (false == skyloupe::text::read(child, decoded.device_id)) return false; continue; }
                if (child.name == "timestamp_us") { if (false == skyloupe::text::read(child, decoded.timestamp_us)) return false; continue; }
                if (child.name == "mode") { if (false == skyloupe::text::read(child, decoded.mode, 4)) return false; continue; }
                if (child.name == "status") { if (false == skyloupe::text::read(child, decoded.status, 3)) return false; continue; }
                if (child.name == "armed") { if (false == skyloupe::text::read(child, decoded.armed, 1)) return false; continue; }
                if (child.name == "callsign") { if (false == skyloupe::text::read_bytes(child, decoded.callsign, 8)) return false; continue; }
                if (child.name == "position") { if (false == decoded.position.from_xml(child)) return false; continue; }
                if (child.name == "battery_mv") { if (false == skyloupe::text::read(child, decoded.battery_mv)) return false; continue; }
            }
            *this = decoded;
            return true;
        }
        std::string to_bson() const {
            skyloupe::bson::Writer w;
            to_bson(w);
            return w.take();
        }
        void to_bson(skyloupe::bson::Writer& w) const {
            w.begin_document();
            w.value("device_id", this->device_id);
            w.value("timestamp_us", this->timestamp_us);
            w.value("mode", this->mode);
            w.value("status", this->status);
            w.value("armed", this->armed);
            w.bytes("callsign", this->callsign, 8);
            w.document("position"); this->position.to_bson(w);
            w.value("battery_mv", this->battery_mv);
            w.end_document();
        }
        bool from_bson(std::string_view bytes) {
            skyloupe::bson::Document doc;
            if (false == skyloupe::bson::parse(bytes, doc)) return false;
            return from_bson(doc);
        }
        bool from_bson(const skyloupe::bson::Document& doc) {
            Telemetry decoded;
            for (const auto& field : doc.elements) {
                if (field.name == "device_id") { if (false == skyloupe::bson::read(field, decoded.device_id)) return false; continue; }
                if (field.name == "timestamp_us") { if (false == skyloupe::bson::read(field, decoded.timestamp_us)) return false; continue; }
                if (field.name == "mode") { if (false == skyloupe::bson::read(field, decoded.mode, 4)) return false; continue; }
                if (field.name == "status") { if (false == skyloupe::bson::read(field, decoded.status, 3)) return false; continue; }
                if (field.name == "armed") { if (false == skyloupe::bson::read(field, decoded.armed, 1)) return false; continue; }
                if (field.name == "callsign") { if (false == skyloupe::bson::read_bytes(field, decoded.callsign, 8)) return false; continue; }
                if (field.name == "position") { if (false == skyloupe::bson::read_document(field, decoded.position)) return false; continue; }
                if (field.name == "battery_mv") { if (false == skyloupe::bson::read(field, decoded.battery_mv)) return false; continue; }
            }
            *this = decoded;
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
