//
//   Copyright (C) 2026 Geoffrey Atkinson
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

// Reads a Telemetry from a BSON document as a MongoDB driver might hand it
// back (telemetry.bson: an _id and a received_at the message does not have,
// members in another order, numbers in other types), writes it back in the
// canonical form, shows a uint64_t above 2^63-1 written as a Decimal128, and
// shows what init(bson:) refuses. See README.md.
//
//   example_bson_swift telemetry.bson out.bson

#if canImport(Skywire)
import Skywire // SWStream and SkywireText, when skywire is a SwiftPM dependency
#endif
import Foundation

func fail(_ message: String) -> Never {
    FileHandle.standardError.write((message + "\n").data(using: .utf8)!)
    exit(1)
}

guard CommandLine.arguments.count == 3 else {
    FileHandle.standardError.write("usage: example_bson_swift INPUT OUTPUT\n".data(using: .utf8)!)
    exit(2)
}
let input = CommandLine.arguments[1]
let output = CommandLine.arguments[2]
guard let data = FileManager.default.contents(atPath: input) else {
    fail("cannot read \(input)")
}

let doc = [UInt8](data)

func contains(_ bytes: [UInt8], _ part: [UInt8]) -> Bool {
    return bytes.count >= part.count
        && (0...(bytes.count - part.count)).contains { Array(bytes[$0..<($0 + part.count)]) == part }
}

// init(bson:) ignores elements the message does not have (_id,
// received_at) and takes any number type that holds the member's value: here
// an int32 device_id and a double battery_mv.
let msg: Telemetry
do {
    msg = try Telemetry(bson: doc)
} catch {
    fail("\(input) is not the BSON form of a Telemetry: \(error)")
}
let callsign = String(decoding: msg.Get_callsign().prefix { $0 != 0 }, as: UTF8.self)
print("device_id \(msg.Get_device_id()), callsign \(callsign), battery_mv \(msg.Get_battery_mv())")

// toBson writes the canonical document: members in tag order, each in the one
// type its member's type maps to (uint32_t device_id as int64, uint16_t
// battery_mv as int32).
let canonical = msg.toBson()
guard FileManager.default.createFile(atPath: output, contents: Data(canonical)) else {
    fail("cannot write \(output)")
}
print("read \(doc.count) bytes, wrote \(canonical.count)")

// A uint64_t above 2^63-1 is written as a Decimal128 (type 0x13), so no tool
// reads it as a negative int64.
do {
    let big = try Telemetry(bson: canonical)
    big.Set_timestamp_us(UInt64.max)
    let bigDoc = big.toBson()
    guard contains(bigDoc, [0x13] + Array("timestamp_us".utf8) + [0]),
          try Telemetry(bson: bigDoc).Equals(big) else {
        fail("timestamp_us \(UInt64.max) did not go through as a Decimal128")
    }
    print("timestamp_us \(UInt64.max) is a Decimal128, and reads back")
} catch {
    fail("timestamp_us \(UInt64.max) did not read back: \(error)")
}

// A document that is not a Telemetry, or holds a value a member cannot, is
// refused: init(bson:) throws SkywireText.Invalid.
let refused: [([UInt8], String)] = [
    (Array(doc.dropLast()), "cut short"),
    (doc + [0], "a byte after the document"),
    ([0x11, 0, 0, 0, 0x10] + Array("status".utf8) + [0, 9, 0, 0, 0, 0], "{status: int32 9}: status has 3 bits"),
    ([0x18, 0, 0, 0, 0x12] + Array("device_id".utf8) + [0] + [UInt8](repeating: 0xff, count: 8) + [0],
     "{device_id: int64 -1}: a uint32_t"),
    ([0x1b, 0, 0, 0, 0x01] + Array("timestamp_us".utf8) + [0, 0, 0, 0, 0, 0, 0, 0xf8, 0x3f, 0],
     "{timestamp_us: double 1.5}: not a whole number"),
]
for (bad, why) in refused {
    if (try? Telemetry(bson: bad)) != nil {
        fail("init(bson:) accepted a document: \(why)")
    }
    print("refused \(why)")
}
