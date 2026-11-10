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
}
