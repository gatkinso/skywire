// This is the generated Swift file

#if canImport(Skywire)
import Skywire
#endif
import Foundation

public final class Telemetry {
    var device_id: UInt32 = 0
    var timestamp_us: UInt64 = 0
    var mode: UInt8 = 0
    var status: UInt8 = 0
    var armed: UInt8 = 0
    var callsign: [UInt8] = [UInt8](repeating: 0, count: 8)
    var position: Position = Position()
    var battery_mv: UInt16 = 0

    public init() {}

    public func serialize(_ stream: SWStream, _ endian: Endianness = .big) -> Bool {
        guard var p = stream.claim(size()) else { return false }
        serialize(into: &p)
        return true
    }

    // Writes the wire representation, size() bytes, at p and moves p past it.
    public func serialize(into p: inout UnsafeMutableRawPointer) {
        SWWire.put(&p, device_id, .big)
        SWWire.put(&p, timestamp_us, .little)
        var unit_0: UInt8 = 0
        unit_0 |= (UInt8(truncatingIfNeeded: mode) & 0xf)
        unit_0 |= (UInt8(truncatingIfNeeded: status) & 0x7) << 4
        unit_0 |= (UInt8(truncatingIfNeeded: armed) & 0x1) << 7
        SWWire.put(&p, unit_0, .big)
        SWWire.put(&p, callsign, 8)
        position.serialize(into: &p)
        SWWire.put(&p, battery_mv, .little)
    }

    public func deserialize(_ stream: SWStream, _ endian: Endianness = .big) -> Bool {
        guard var p = stream.consume(size()) else { return false }
        deserialize(from: &p)
        return true
    }

    // Reads the wire representation, size() bytes, from p and moves p past it.
    public func deserialize(from p: inout UnsafeRawPointer) {
        device_id = SWWire.get(&p, .big)
        timestamp_us = SWWire.get(&p, .little)
        let unit_0: UInt8 = SWWire.get(&p, .big)
        mode = UInt8(truncatingIfNeeded: unit_0 << 4) >> 4
        status = UInt8(truncatingIfNeeded: unit_0 << 1) >> 5
        armed = UInt8(truncatingIfNeeded: unit_0) >> 7
        SWWire.get(&p, &callsign, 8)
        position.deserialize(from: &p)
        battery_mv = SWWire.get(&p, .little)
    }

    public func size() -> Int {
        return (
            MemoryLayout<UInt32>.size +
            MemoryLayout<UInt64>.size +
            MemoryLayout<UInt8>.size +
            8 +
            position.size() +
            MemoryLayout<UInt16>.size
        )
    }

    public func Equals(_ other: Telemetry) -> Bool {
        return (
            device_id == other.device_id &&
            timestamp_us == other.timestamp_us &&
            mode == other.mode &&
            status == other.status &&
            armed == other.armed &&
            callsign == other.callsign &&
            position.Equals(other.position) &&
            battery_mv == other.battery_mv
        )
    }

    public func Get_device_id() -> UInt32 { return device_id }
    public func Get_timestamp_us() -> UInt64 { return timestamp_us }
    public func Get_mode() -> UInt8 { return mode }
    public func Get_status() -> UInt8 { return status }
    public func Get_armed() -> UInt8 { return armed }
    public func Get_callsign() -> [UInt8] { return callsign }
    public func Get_position() -> Position { return position }
    public func Get_battery_mv() -> UInt16 { return battery_mv }
    public func Set_device_id(_ newval: UInt32) { device_id = newval }
    public func Set_timestamp_us(_ newval: UInt64) { timestamp_us = newval }
    public func Set_mode(_ newval: UInt8) { mode = (newval << 4) >> 4 }
    public func Set_status(_ newval: UInt8) { status = (newval << 5) >> 5 }
    public func Set_armed(_ newval: UInt8) { armed = (newval << 7) >> 7 }
    public func Set_callsign(_ newval: [UInt8]) { callsign = SWStream.fit(newval, 8) }
    public func Set_position(_ newval: Position) { position = newval }
    public func Set_battery_mv(_ newval: UInt16) { battery_mv = newval }

    /// Returns the JSON form of this message (docs/text-formats.md).
    public func toJson() -> String {
        let w = SkywireText.JsonWriter()
        writeJson(w)
        return w.finish()
    }

    /// Reads the message from its JSON form (docs/text-formats.md): members
    /// in any order, missing ones zero, unknown ones ignored. Throws
    /// SkywireText.Invalid if json is not JSON, or a member's value is not one
    /// it can hold.
    public convenience init(json: String) throws {
        self.init()
        try readJson(SkywireText.parseJson(json))
    }

    /// Reads the message from its JSON form in UTF-8, as init(json: String)
    /// does.
    public convenience init(json: [UInt8]) throws {
        self.init()
        try readJson(SkywireText.parseJson(json))
    }

    /// Writes this message as a JSON object, as toJson and a containing
    /// message do.
    public func writeJson(_ w: SkywireText.JsonWriter) {
        w.beginObject()
        w.u32("device_id", self.device_id)
        w.u64("timestamp_us", self.timestamp_us)
        w.i32("mode", Int32(self.mode))
        w.i32("status", Int32(self.status))
        w.i32("armed", Int32(self.armed))
        w.bytes("callsign", self.callsign)
        w.key("position")
        self.position.writeJson(w)
        w.i32("battery_mv", Int32(self.battery_mv))
        w.endObject()
    }

    /// Reads this message's members from a JSON object, as init(json:) and a
    /// containing message do; members missing from it are left as they are.
    public func readJson(_ value: SkywireText.Json) throws {
        for m in try value.members() {
            switch m.name {
            case "device_id":
                self.device_id = try UInt32(truncatingIfNeeded: m.value.integer("uint32_t", 0))
            case "timestamp_us":
                self.timestamp_us = try UInt64(truncatingIfNeeded: m.value.integer("uint64_t", 0))
            case "mode":
                self.mode = try UInt8(truncatingIfNeeded: m.value.integer("uint8_t", 4))
            case "status":
                self.status = try UInt8(truncatingIfNeeded: m.value.integer("uint8_t", 3))
            case "armed":
                self.armed = try UInt8(truncatingIfNeeded: m.value.integer("uint8_t", 1))
            case "callsign":
                try m.value.bytes(&self.callsign)
            case "position":
                self.position = Position()
                try self.position.readJson(m.value)
            case "battery_mv":
                self.battery_mv = try UInt16(truncatingIfNeeded: m.value.integer("uint16_t", 0))
            default:
                break
            }
        }
    }

    /// Returns the XML form of this message (docs/text-formats.md).
    public func toXml() -> String {
        let w = SkywireText.XmlWriter()
        writeXml(w, "Telemetry")
        return w.finish()
    }

    /// Reads the message from its XML form (docs/text-formats.md): members in
    /// any order, missing ones zero, unknown ones ignored. Throws
    /// SkywireText.Invalid if xml is not XML with the root element
    /// Telemetry, or a member's value is not one it can hold.
    public convenience init(xml: String) throws {
        self.init()
        try readXml(SkywireText.parseXml(xml, "Telemetry"))
    }

    /// Reads the message from its XML form in UTF-8, as init(xml: String)
    /// does.
    public convenience init(xml: [UInt8]) throws {
        self.init()
        try readXml(SkywireText.parseXml(xml, "Telemetry"))
    }

    /// Writes this message as the XML element named element, as toXml and a
    /// containing message do.
    public func writeXml(_ w: SkywireText.XmlWriter, _ element: String) {
        w.open(element)
        w.u32("device_id", self.device_id)
        w.u64("timestamp_us", self.timestamp_us)
        w.i32("mode", Int32(self.mode))
        w.i32("status", Int32(self.status))
        w.i32("armed", Int32(self.armed))
        w.bytes("callsign", self.callsign)
        self.position.writeXml(w, "position")
        w.i32("battery_mv", Int32(self.battery_mv))
        w.close(element)
    }

    /// Reads this message's members from an XML element, as init(xml:) and a
    /// containing message do; members missing from it are left as they are.
    public func readXml(_ node: SkywireText.XmlNode) throws {
        for m in try node.members() {
            switch m.name {
            case "device_id":
                self.device_id = try UInt32(truncatingIfNeeded: m.integer("uint32_t", 0))
            case "timestamp_us":
                self.timestamp_us = try UInt64(truncatingIfNeeded: m.integer("uint64_t", 0))
            case "mode":
                self.mode = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 4))
            case "status":
                self.status = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 3))
            case "armed":
                self.armed = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 1))
            case "callsign":
                try m.bytes(&self.callsign)
            case "position":
                self.position = Position()
                try self.position.readXml(m)
            case "battery_mv":
                self.battery_mv = try UInt16(truncatingIfNeeded: m.integer("uint16_t", 0))
            default:
                break
            }
        }
    }

    /// Returns the BSON document of this message (docs/text-formats.md).
    public func toBson() -> [UInt8] {
        let w = SkywireText.BsonWriter()
        writeBson(w)
        return w.finish()
    }

    /// Reads the message from bson, which must be exactly one BSON document
    /// (docs/text-formats.md): elements in any order, missing ones zero,
    /// unknown ones ignored. Throws SkywireText.Invalid if bson is not a BSON
    /// document, or an element's value is not one its member can hold.
    public convenience init(bson: [UInt8]) throws {
        self.init()
        try readBson(SkywireText.parseBson(bson))
    }

    /// Writes this message as a BSON document, as toBson and a containing
    /// message do.
    public func writeBson(_ w: SkywireText.BsonWriter) {
        w.beginDocument()
        w.u32("device_id", self.device_id)
        w.u64("timestamp_us", self.timestamp_us)
        w.i32("mode", Int32(self.mode))
        w.i32("status", Int32(self.status))
        w.i32("armed", Int32(self.armed))
        w.bytes("callsign", self.callsign)
        w.document("position")
        self.position.writeBson(w)
        w.i32("battery_mv", Int32(self.battery_mv))
        w.endDocument()
    }

    /// Reads this message's members from a BSON document's elements, as
    /// init(bson:) and a containing message do; members missing from it are
    /// left as they are.
    public func readBson(_ doc: [SkywireText.BsonElement]) throws {
        for m in doc {
            switch m.name {
            case "device_id":
                self.device_id = try UInt32(truncatingIfNeeded: m.integer("uint32_t", 0))
            case "timestamp_us":
                self.timestamp_us = try UInt64(truncatingIfNeeded: m.integer("uint64_t", 0))
            case "mode":
                self.mode = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 4))
            case "status":
                self.status = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 3))
            case "armed":
                self.armed = try UInt8(truncatingIfNeeded: m.integer("uint8_t", 1))
            case "callsign":
                try m.bytes(&self.callsign)
            case "position":
                self.position = Position()
                try self.position.readBson(m.document())
            case "battery_mv":
                self.battery_mv = try UInt16(truncatingIfNeeded: m.integer("uint16_t", 0))
            default:
                break
            }
        }
    }
}
