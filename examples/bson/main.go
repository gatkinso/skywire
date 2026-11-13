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

// Reads a Telemetry from a BSON document as a MongoDB driver might hand it
// back (telemetry.bson: an _id and a received_at the message does not have,
// members in another order, numbers in other types), writes it back in the
// canonical form, shows a uint64_t above 2^63-1 written as a Decimal128, and
// shows what UnmarshalBSON refuses. See README.md.
//
//	example_bson_go telemetry.bson out.bson
package main

import (
	"bytes"
	"fmt"
	"math"
	"os"
)

func fail(format string, args ...any) {
	fmt.Fprintf(os.Stderr, format+"\n", args...)
	os.Exit(1)
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintln(os.Stderr, "usage: example_bson_go INPUT OUTPUT")
		os.Exit(2)
	}
	doc, err := os.ReadFile(os.Args[1])
	if err != nil {
		fail("cannot read %s: %v", os.Args[1], err)
	}

	// UnmarshalBSON ignores elements the message does not have (_id,
	// received_at) and takes any number type that holds the member's value:
	// here an int32 device_id and a double battery_mv.
	var msg Telemetry
	if err := msg.UnmarshalBSON(doc); err != nil {
		fail("%s is not the BSON form of a Telemetry: %v", os.Args[1], err)
	}
	callsign := msg.Get_callsign()
	fmt.Printf("device_id %d, callsign %s, battery_mv %d\n", msg.Get_device_id(),
		bytes.TrimRight(callsign[:], "\x00"), msg.Get_battery_mv())

	// MarshalBSON writes the canonical document: members in tag order, each
	// in the one type its member's type maps to (uint32_t device_id as int64,
	// uint16_t battery_mv as int32).
	canonical, err := msg.MarshalBSON()
	if err != nil {
		fail("MarshalBSON failed: %v", err)
	}
	if err := os.WriteFile(os.Args[2], canonical, 0o644); err != nil {
		fail("cannot write %s: %v", os.Args[2], err)
	}
	fmt.Printf("read %d bytes, wrote %d\n", len(doc), len(canonical))

	// A uint64_t above 2^63-1 is written as a Decimal128 (type 0x13), so no
	// tool reads it as a negative int64.
	big := msg
	big.Set_timestamp_us(math.MaxUint64)
	bigDoc, err := big.MarshalBSON()
	if err != nil {
		fail("MarshalBSON failed: %v", err)
	}
	var back Telemetry
	if !bytes.Contains(bigDoc, []byte("\x13timestamp_us\x00")) || back.UnmarshalBSON(bigDoc) != nil ||
		!back.Equals(&big) {
		fail("timestamp_us %d did not go through as a Decimal128", uint64(math.MaxUint64))
	}
	fmt.Printf("timestamp_us %d is a Decimal128, and reads back\n", uint64(math.MaxUint64))

	// A document that is not a Telemetry, or holds a value a member cannot,
	// is refused, and the message is left as it was.
	refused := []struct {
		doc []byte
		why string
	}{
		{doc[:len(doc)-1], "cut short"},
		{append(append([]byte{}, doc...), 0), "a byte after the document"},
		{[]byte("\x11\x00\x00\x00\x10status\x00\x09\x00\x00\x00\x00"), "{status: int32 9}: status has 3 bits"},
		{[]byte("\x18\x00\x00\x00\x12device_id\x00\xff\xff\xff\xff\xff\xff\xff\xff\x00"),
			"{device_id: int64 -1}: a uint32_t"},
		{[]byte("\x1b\x00\x00\x00\x01timestamp_us\x00\x00\x00\x00\x00\x00\x00\xf8\x3f\x00"),
			"{timestamp_us: double 1.5}: not a whole number"},
	}
	for _, bad := range refused {
		before := msg
		if err := msg.UnmarshalBSON(bad.doc); err == nil || !msg.Equals(&before) {
			fail("UnmarshalBSON accepted a document: %s", bad.why)
		}
		fmt.Printf("refused %s\n", bad.why)
	}
}
