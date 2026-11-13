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

// Reads a Telemetry from XML written by another program (telemetry.xml: a
// declaration, comments, indentation, CDATA, an entity, a string in
// hexadecimal and an element the message does not have), writes it back in
// the canonical form, and shows what init(xml:) refuses. See README.md.
//
//   example_xml_swift telemetry.xml out.xml

#if canImport(Skywire)
import Skywire // SWStream and SkywireText, when skywire is a SwiftPM dependency
#endif
import Foundation

func fail(_ message: String) -> Never {
    FileHandle.standardError.write((message + "\n").data(using: .utf8)!)
    exit(1)
}

guard CommandLine.arguments.count == 3 else {
    FileHandle.standardError.write("usage: example_xml_swift INPUT OUTPUT\n".data(using: .utf8)!)
    exit(2)
}
let input = CommandLine.arguments[1]
let output = CommandLine.arguments[2]
guard let data = FileManager.default.contents(atPath: input) else {
    fail("cannot read \(input)")
}

// The root element must be the message's name; members may come in any
// order, and an element the message does not have (firmware) is ignored.
let msg: Telemetry
do {
    msg = try Telemetry(xml: [UInt8](data))
} catch {
    fail("\(input) is not the XML form of a Telemetry: \(error)")
}
let hex = msg.Get_callsign().prefix(5).map { String(format: "%02x", $0) }.joined(separator: " ")
print("device_id \(msg.Get_device_id()), callsign bytes \(hex), altitude_m "
    + String(format: "%.1f", Double(msg.Get_position().Get_altitude_m())))

// toXml writes the canonical form: no declaration, no whitespace, and the
// callsign in hexadecimal again, as it holds a byte that is not printable
// ASCII.
let canonical = msg.toXml()
guard FileManager.default.createFile(atPath: output, contents: Data(canonical.utf8)) else {
    fail("cannot write \(output)")
}
print(canonical)

// Input that is not a Telemetry, or holds a value a member cannot, is
// refused: init(xml:) throws SkywireText.Invalid.
let refused = [
    #"<!DOCTYPE Telemetry [<!ENTITY x "y">]><Telemetry/>"#,                   // no document type declarations
    #"<Position/>"#,                                                          // not a Telemetry
    #"<Telemetry><mode>5</Telemetry>"#,                                       // not well-formed
    #"<Telemetry><status>8</status></Telemetry>"#,                            // status has 3 bits
    #"<Telemetry><callsign encoding="base64">U0tZMQ==</callsign></Telemetry>"#, // hex only
]
for bad in refused {
    if (try? Telemetry(xml: bad)) != nil {
        fail("init(xml:) accepted \(bad)")
    }
    print("refused \(bad)")
}
