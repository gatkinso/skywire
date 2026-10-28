// The ECMAScript spec leaves the bits of a NaN that DataView reads or writes
// up to the engine, so a relay could in principle change a NaN's payload.
// This pins what Node does today: a quiet NaN's payload survives a round
// trip, for float and double alike. (A signaling float NaN comes back quiet:
// widening it to a double sets the quiet bit.) If an engine change breaks
// this, the README's note on NaN payloads needs updating.

import { test } from "node:test";
import assert from "node:assert/strict";
import { JazzyMcJsface } from "./JazzyMcJsface.js";

function hex(bytes) {
  return Array.from(bytes, (b) => b.toString(16).padStart(2, "0")).join(" ");
}

test("JazzyMcJsface NaNPayload", () => {
  const wire = new JazzyMcJsface().serialize();
  const view = new DataView(wire.buffer);
  view.setUint32(0, 0x7fc00001, true);             // a_float_le
  view.setBigUint64(4, 0x7ff8000000000001n, false); // b_double_be
  const msg = JazzyMcJsface.fromBytes(wire);
  assert.ok(Number.isNaN(msg.a_float_le) && Number.isNaN(msg.b_double_be));
  assert.equal(hex(msg.serialize()), hex(wire));
});
