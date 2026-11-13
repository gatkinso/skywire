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

// The Rust driver for skywire_text_diff: prints what the generated Rust ring
// message makes of each input, as oracle.cpp does for C++. text_diff.cmake
// builds it as the root of a crate beside the generated modules.
//
//     driver <inputs>

// The driver uses only some of what the modules define.
#![allow(dead_code)]

mod ringy_inner;
mod ringy_mc_ringface;
mod skywire_text;

use ringy_mc_ringface::RingyMcRingface;
use std::io::Write;

fn line(out: &mut impl Write, tag: &str, ok: bool, data: &[u8]) {
    let mut text = String::from(tag);
    if ok {
        text.push(' ');
        for b in data {
            text.push_str(&format!("{b:02x}"));
        }
    } else {
        text.push_str(" -");
    }
    text.push('\n');
    out.write_all(text.as_bytes()).expect("write");
}

fn main() {
    let path = std::env::args().nth(1).expect("usage: driver <inputs>");
    let raw = std::fs::read(path).expect("read the inputs");
    let stdout = std::io::stdout();
    let mut out = std::io::BufWriter::new(stdout.lock());
    let mut at = 0;
    while at < raw.len() {
        let mut length = [0; 4];
        length.copy_from_slice(&raw[at..at + 4]);
        let n = u32::from_le_bytes(length) as usize;
        let data = &raw[at + 4..at + 4 + n];
        at += 4 + n;

        let json = RingyMcRingface::from_json(data);
        let ok = json.is_ok();
        line(&mut out, "J", ok, json.unwrap_or_default().to_json().as_bytes());
        let xml = RingyMcRingface::from_xml(data);
        let ok = xml.is_ok();
        line(&mut out, "X", ok, xml.unwrap_or_default().to_xml().as_bytes());
        let bson = RingyMcRingface::from_bson(data);
        let ok = bson.is_ok();
        line(&mut out, "B", ok, &bson.unwrap_or_default().to_bson());
    }
    out.flush().expect("flush");
}
