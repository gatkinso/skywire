// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//   example_swift telemetry.bin

import Foundation

func fail(_ message: String) -> Never {
    FileHandle.standardError.write((message + "\n").data(using: .utf8)!)
    exit(1)
}

guard CommandLine.arguments.count == 2 else {
    FileHandle.standardError.write("usage: example_swift FILE\n".data(using: .utf8)!)
    exit(2)
}
let path = CommandLine.arguments[1]

let position = Position()
position.Set_latitude(47.6062)
position.Set_longitude(-122.3321)
position.Set_altitude_m(56.5)

let sent = Telemetry()
sent.Set_device_id(4207)
sent.Set_timestamp_us(1_760_959_800_123_456)
sent.Set_mode(5)
sent.Set_status(11) // status has 3 bits: 11 is stored as 3
sent.Set_armed(1)
sent.Set_callsign(Array("SKY1".utf8)) // NUL-padded to 8 bytes
sent.Set_position(position)
sent.Set_battery_mv(3870)

// Serialize into a stream, then write the stream's bytes to the file.
let stream = SWStream()
guard sent.serialize(stream), let bytes = stream.read(stream.remaining()) else {
    fail("serialize failed")
}
guard FileManager.default.createFile(atPath: path, contents: Data(bytes)) else {
    fail("cannot write \(path)")
}

// Read the file back into the stream and deserialize a second message.
guard let back = FileManager.default.contents(atPath: path) else {
    fail("cannot open \(path)")
}
stream.write([UInt8](back))
let received = Telemetry()
guard received.deserialize(stream) else {
    fail("deserialize failed")
}

guard received.Equals(sent) else {
    fail("the message read back is different")
}

let callsign = String(decoding: received.Get_callsign().prefix { $0 != 0 }, as: UTF8.self)
let p = received.Get_position()
print("wrote and read back \(back.count) bytes (size() = \(received.size()))")
print("device_id \(received.Get_device_id()), callsign \(callsign), status \(received.Get_status()), "
    + "position \(p.Get_latitude()), \(p.Get_longitude()), \(p.Get_altitude_m()) m")
