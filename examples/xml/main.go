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

// Reads a Telemetry from XML written by another program (telemetry.xml: a
// declaration, comments, indentation, CDATA, an entity, a string in
// hexadecimal and an element the message does not have), writes it back in
// the canonical form, and shows what FromXML refuses. See README.md.
//
//	example_xml_go telemetry.xml out.xml
package main

import (
	"fmt"
	"os"
)

func fail(format string, args ...any) {
	fmt.Fprintf(os.Stderr, format+"\n", args...)
	os.Exit(1)
}

func main() {
	if len(os.Args) != 3 {
		fmt.Fprintln(os.Stderr, "usage: example_xml_go INPUT OUTPUT")
		os.Exit(2)
	}
	text, err := os.ReadFile(os.Args[1])
	if err != nil {
		fail("cannot read %s: %v", os.Args[1], err)
	}

	// The root element must be the message's name; members may come in any
	// order, and an element the message does not have (firmware) is ignored.
	var msg Telemetry
	if err := msg.FromXML(text); err != nil {
		fail("%s is not the XML form of a Telemetry: %v", os.Args[1], err)
	}
	callsign := msg.Get_callsign()
	position := msg.Get_position()
	fmt.Printf("device_id %d, callsign bytes % x, altitude_m %.1f\n", msg.Get_device_id(), callsign[:5],
		position.Get_altitude_m())

	// ToXML writes the canonical form: no declaration, no whitespace, and the
	// callsign in hexadecimal again, as it holds a byte that is not printable
	// ASCII.
	canonical := msg.ToXML()
	if err := os.WriteFile(os.Args[2], canonical, 0o644); err != nil {
		fail("cannot write %s: %v", os.Args[2], err)
	}
	fmt.Printf("%s\n", canonical)

	// Input that is not a Telemetry, or holds a value a member cannot, is
	// refused, and the message is left as it was.
	refused := []string{
		`<!DOCTYPE Telemetry [<!ENTITY x "y">]><Telemetry/>`, // no document type declarations
		`<Position/>`,                               // not a Telemetry
		`<Telemetry><mode>5</Telemetry>`,            // not well-formed
		`<Telemetry><status>8</status></Telemetry>`, // status has 3 bits
		`<Telemetry><callsign encoding="base64">U0tZMQ==</callsign></Telemetry>`, // hex only
	}
	for _, bad := range refused {
		before := msg
		if err := msg.FromXML([]byte(bad)); err == nil || !msg.Equals(&before) {
			fail("FromXML accepted %s", bad)
		}
		fmt.Printf("refused %s\n", bad)
	}
}
