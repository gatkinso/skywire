// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//     node main.js telemetry.bin

import { readFileSync, writeFileSync } from "node:fs";
import { Position } from "./Position.js";
import { Telemetry } from "./Telemetry.js";

function main(args) {
  if (args.length !== 1) {
    console.error("usage: node main.js FILE");
    return 2;
  }
  const path = args[0];

  const position = new Position();
  position.latitude = 47.6062;
  position.longitude = -122.3321;
  position.altitude_m = 56.5;

  const sent = new Telemetry();
  sent.device_id = 4207;
  sent.timestamp_us = 1760959800123456n; // uint64_t is a BigInt
  sent.mode = 5;
  sent.status = 11; // status has 3 bits: 11 is stored as 3
  sent.armed = 1;
  sent.callsign = "SKY1"; // UTF-8, NUL-padded to 8 bytes
  sent.position = position;
  sent.battery_mv = 3870;

  // Serialize to Telemetry.SIZE bytes and write them to the file.
  writeFileSync(path, sent.serialize());

  // Read the file back and deserialize a second message. A Node Buffer is a
  // Uint8Array, so it goes straight in; fromBytes throws a RangeError if the
  // input is short.
  const back = readFileSync(path);
  const received = Telemetry.fromBytes(back);

  if (!received.equals(sent)) {
    console.error("the message read back is different");
    return 1;
  }

  const p = received.position;
  console.log(`wrote and read back ${back.length} bytes (size() = ${received.size()})`);
  console.log(`device_id ${received.device_id}, callsign ${received.callsignText}, status ${received.status}, ` +
              `position ${p.latitude.toFixed(4)}, ${p.longitude.toFixed(4)}, ${p.altitude_m.toFixed(1)} m`);

  // The same message as JSON, XML and BSON (docs/text-formats.md): each is
  // written beside the file, read back and checked. A from method throws a
  // SyntaxError on input it refuses.
  const base = path.endsWith(".bin") ? path.slice(0, -4) : path;
  writeFileSync(base + ".json", sent.toJsonText());
  writeFileSync(base + ".xml", sent.toXml());
  writeFileSync(base + ".bson", sent.toBson());
  const copies = [
    Telemetry.fromJsonText(readFileSync(base + ".json")),
    Telemetry.fromXml(readFileSync(base + ".xml")),
    Telemetry.fromBson(readFileSync(base + ".bson")),
  ];
  if (!copies.every((copy) => copy.equals(sent))) {
    console.error("a JSON, XML or BSON copy is different");
    return 1;
  }
  console.log(sent.toJsonText());
  return 0;
}

process.exitCode = main(process.argv.slice(2));
