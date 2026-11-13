// Copyright (C) 2026 Geoffrey Atkinson
//
// Licensed under the Apache License, Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// A copy of the License is located at
//
// https://www.apache.org/licenses/LICENSE-2.0.html
//
// or in the "license" file accompanying this file. This file is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing
// permissions and limitations under the License.

// The Swift driver for skywire_text_diff: prints what the generated Swift
// ring message makes of each input, as oracle.cpp does for C++.
// text_diff.cmake builds it, as main.swift, with SWStream.swift,
// SkywireText.swift and the generated classes.
//
//     driver <inputs>

import Foundation

let hexDigits = Array("0123456789abcdef".utf8)
var out: [UInt8] = []

func line(_ tag: String, _ data: [UInt8]?) {
    out.append(contentsOf: Array(tag.utf8))
    if let data = data {
        out.append(UInt8(ascii: " "))
        for b in data {
            out.append(hexDigits[Int(b >> 4)])
            out.append(hexDigits[Int(b & 15)])
        }
    } else {
        out.append(contentsOf: Array(" -".utf8))
    }
    out.append(UInt8(ascii: "\n"))
}

let raw = [UInt8](FileManager.default.contents(atPath: CommandLine.arguments[1])!)
var at = 0
while at < raw.count {
    let n = Int(raw[at]) | Int(raw[at + 1]) << 8 | Int(raw[at + 2]) << 16 | Int(raw[at + 3]) << 24
    let data = Array(raw[(at + 4)..<(at + 4 + n)])
    at += 4 + n

    line("J", (try? RingyMcRingface(json: data)).map { Array($0.toJson().utf8) })
    line("X", (try? RingyMcRingface(xml: data)).map { Array($0.toXml().utf8) })
    line("B", (try? RingyMcRingface(bson: data)).map { $0.toBson() })
}
FileHandle.standardOutput.write(Data(out))
