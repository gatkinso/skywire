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

// Reads a Telemetry from JSON written by hand (telemetry.json: indented,
// members in another order, one missing and one the message does not have),
// writes it back in the canonical form, and shows what FromJSON refuses. See
// README.md.
//
//	example_json_go telemetry.json out.json
package main

import (
	"bytes"
	"fmt"
	"os"
)

func fail(format string, args ...any) {
	fmt.Fprintf(os.Stderr, format+"\n", args...)
	os.Exit(1)
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintln(os.Stderr, "usage: example_json_go INPUT OUTPUT")
		os.Exit(2)
	}
	text, err := os.ReadFile(os.Args[1])
	if err != nil {
		fail("cannot read %s: %v", os.Args[1], err)
	}

	// FromJSON takes members in any order, leaves a missing one (battery_mv)
	// zero and ignores one the message does not have (firmware). A 64-bit
	// integer may be a JSON string, as ToJSON writes it.
	var msg Telemetry
	if err := msg.FromJSON(text); err != nil {
		fail("%s is not the JSON form of a Telemetry: %v", os.Args[1], err)
	}
	callsign := msg.Get_callsign()
	fmt.Printf("device_id %d, callsign %s, timestamp_us %d, battery_mv %d\n", msg.Get_device_id(),
		bytes.TrimRight(callsign[:], "\x00"), msg.Get_timestamp_us(), msg.Get_battery_mv())

	// ToJSON writes the canonical form: no whitespace, members in tag order.
	canonical := msg.ToJSON()
	if err := os.WriteFile(os.Args[2], canonical, 0o644); err != nil {
		fail("cannot write %s: %v", os.Args[2], err)
	}
	fmt.Printf("%s\n", canonical)

	// Input that is not a Telemetry, or holds a value a member cannot, is
	// refused, and the message is left as it was.
	refused := []string{
		`{"status": 9}`,                     // status has 3 bits
		`{"device_id": -1}`,                 // a uint32_t
		`{"timestamp_us": 1.5}`,             // not a whole number
		`{"callsign": "TOO-LONG-CALLSIGN"}`, // more than 8 bytes
		`{"armed": true}`,                   // not a number
		`{"mode": 5,}`,                      // not JSON
	}
	for _, bad := range refused {
		before := msg
		if err := msg.FromJSON([]byte(bad)); err == nil || !msg.Equals(&before) {
			fail("FromJSON accepted %s", bad)
		}
		fmt.Printf("refused %s\n", bad)
	}
}
