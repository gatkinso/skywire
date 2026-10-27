// Times the generated JavaScript code for the ring message. See ../README.md.
//
//   node bench.js RingyMcRingface.bin

import { readFileSync } from "node:fs";
import { RingyMcRingface } from "./RingyMcRingface.js";

// The first rounds let V8 reach its optimized code; the best round is
// reported.
const ROUNDS = 8;
const ITERS = 1_000_000;

let sink = 0;

// Best time per call, in nanoseconds, over ROUNDS rounds of ITERS calls.
function bestNs(body) {
  let best = Infinity;
  for (let r = 0; r < ROUNDS; r++) {
    const start = process.hrtime.bigint();
    for (let i = 0; i < ITERS; i++) {
      sink += body();
    }
    best = Math.min(best, Number(process.hrtime.bigint() - start) / ITERS);
  }
  return best;
}

function main(args) {
  if (args.length !== 1) {
    console.error("usage: node bench.js RingyMcRingface.bin");
    return 2;
  }
  const bin = new Uint8Array(readFileSync(args[0]));

  // The message must decode and re-encode to the same bytes.
  const m = RingyMcRingface.fromBytes(bin);
  const again = m.serialize();
  if (again.length !== bin.length || again.some((b, i) => b !== bin[i])) {
    console.error("serialize does not reproduce the input");
    return 1;
  }

  // A DataView over a reused buffer, as a nested message is given.
  const buf = new DataView(new ArrayBuffer(4096));
  const m2 = new RingyMcRingface();
  const ser = bestNs(() => m.serializeInto(buf, 0));
  const serArray = bestNs(() => m.serialize()[0]);
  const de = bestNs(() => m2.deserializeFrom(bin, 0));

  console.log(`RESULT serialize ${ser.toFixed(1)}`);
  console.log(`RESULT deserialize ${de.toFixed(1)}`);
  console.log(`NOTE serialize() to a new Uint8Array ${serArray.toFixed(1)} ns; Node ${process.versions.node}`);
  if (sink === 42) {
    console.log();
  }
  return 0;
}

process.exitCode = main(process.argv.slice(2));
