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

// Reads a Telemetry from JSON written by hand (telemetry.json: indented,
// members in another order, one missing and one the message does not have),
// writes it back in the canonical form, and shows what init(json:) refuses.
// See README.md.
//
//   example_json_swift telemetry.json out.json

#if canImport(Skywire)
import Skywire // SWStream and SkywireText, when skywire is a SwiftPM dependency
#endif
import Foundation

func fail(_ message: String) -> Never {
    FileHandle.standardError.write((message + "\n").data(using: .utf8)!)
    exit(1)
}

guard CommandLine.arguments.count == 3 else {
    FileHandle.standardError.write("usage: example_json_swift INPUT OUTPUT\n".data(using: .utf8)!)
    exit(2)
}
let input = CommandLine.arguments[1]
let output = CommandLine.arguments[2]
guard let data = FileManager.default.contents(atPath: input) else {
    fail("cannot read \(input)")
}

// init(json:) takes members in any order, leaves a missing one (battery_mv)
// zero and ignores one the message does not have (firmware). A 64-bit
// integer may be a JSON string, as toJson writes it.
let msg: Telemetry
do {
    msg = try Telemetry(json: [UInt8](data))
} catch {
    fail("\(input) is not the JSON form of a Telemetry: \(error)")
}
let callsign = String(decoding: msg.Get_callsign().prefix { $0 != 0 }, as: UTF8.self)
print("device_id \(msg.Get_device_id()), callsign \(callsign), timestamp_us \(msg.Get_timestamp_us()), "
    + "battery_mv \(msg.Get_battery_mv())")

// toJson writes the canonical form: no whitespace, members in tag order.
let canonical = msg.toJson()
guard FileManager.default.createFile(atPath: output, contents: Data(canonical.utf8)) else {
    fail("cannot write \(output)")
}
print(canonical)

// Input that is not a Telemetry, or holds a value a member cannot, is
// refused: init(json:) throws SkywireText.Invalid.
let refused = [
    #"{"status": 9}"#,                     // status has 3 bits
    #"{"device_id": -1}"#,                 // a uint32_t
    #"{"timestamp_us": 1.5}"#,             // not a whole number
    #"{"callsign": "TOO-LONG-CALLSIGN"}"#, // more than 8 bytes
    #"{"armed": true}"#,                   // not a number
    #"{"mode": 5,}"#,                      // not JSON
]
for bad in refused {
    if (try? Telemetry(json: bad)) != nil {
        fail("init(json:) accepted \(bad)")
    }
    print("refused \(bad)")
}
