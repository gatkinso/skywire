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

// The JavaScript driver for skywire_text_diff: prints what the generated
// JavaScript ring message makes of each input, as oracle.cpp does for C++.
//
//     node driver.mjs <generated dir> <inputs>

import { readFileSync, writeSync } from "node:fs";
import { pathToFileURL } from "node:url";
import { join } from "node:path";

const [dir, path] = process.argv.slice(2);
const { RingyMcRingface } = await import(pathToFileURL(join(dir, "RingyMcRingface.js")).href);
const encoder = new TextEncoder();

function attempt(read, write) {
  let msg;
  try {
    msg = read();
  } catch (e) {
    if (e instanceof SyntaxError) {
      return null;
    }
    throw e;
  }
  return write(msg);
}

const raw = readFileSync(path);
const out = [];
let at = 0;
while (at < raw.length) {
  const n = raw.readUInt32LE(at);
  const data = new Uint8Array(raw.buffer, raw.byteOffset + at + 4, n);
  at += 4 + n;

  const lines = [
    ["J", attempt(() => RingyMcRingface.fromJsonText(data), (m) => encoder.encode(m.toJsonText()))],
    ["X", attempt(() => RingyMcRingface.fromXml(data), (m) => encoder.encode(m.toXml()))],
    ["B", attempt(() => RingyMcRingface.fromBson(data), (m) => m.toBson())],
  ];
  for (const [tag, bytes] of lines) {
    out.push(tag + (bytes === null ? " -" : " " + Buffer.from(bytes).toString("hex")));
  }
}
const text = Buffer.from(out.join("\n") + "\n");
for (let done = 0; done < text.length;) {
  done += writeSync(1, text, done);
}
