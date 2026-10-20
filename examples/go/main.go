// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//	example_go telemetry.bin
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
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: example_go FILE")
		os.Exit(2)
	}
	path := os.Args[1]

	var position Position
	position.Set_latitude(47.6062)
	position.Set_longitude(-122.3321)
	position.Set_altitude_m(56.5)

	var sent Telemetry
	sent.Set_device_id(4207)
	sent.Set_timestamp_us(1760959800123456)
	sent.Set_mode(5)
	sent.Set_status(11) // status has 3 bits: 11 is stored as 3
	sent.Set_armed(1)
	sent.Set_callsign([]byte("SKY1")) // NUL-padded to 8 bytes
	sent.Set_position(position)
	sent.Set_battery_mv(3870)

	// Serialize to TelemetrySize bytes and write them to the file.
	if err := os.WriteFile(path, sent.Serialize(), 0o644); err != nil {
		fail("cannot write %s: %v", path, err)
	}

	// Read the file back and deserialize a second message.
	back, err := os.ReadFile(path)
	if err != nil {
		fail("cannot open %s: %v", path, err)
	}
	var received Telemetry
	if err := received.Deserialize(back); err != nil {
		fail("deserialize failed: %v", err)
	}

	if !received.Equals(&sent) {
		fail("the message read back is different")
	}

	callsign := received.Get_callsign()
	p := received.Get_position()
	fmt.Printf("wrote and read back %d bytes (Size() = %d)\n", len(back), received.Size())
	fmt.Printf("device_id %d, callsign %s, status %d, position %.4f, %.4f, %.1f m\n",
		received.Get_device_id(), bytes.TrimRight(callsign[:], "\x00"), received.Get_status(),
		p.Get_latitude(), p.Get_longitude(), p.Get_altitude_m())
}
