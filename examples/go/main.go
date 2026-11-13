// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//	example_go telemetry.bin
package main

import (
	"bytes"
	"fmt"
	"os"
	"strings"
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

	// The same message as JSON, XML and BSON (docs/text-formats.md): each is
	// written beside the file, read back and checked. A From method returns
	// an error, leaving the message unchanged, on input it refuses.
	base := strings.TrimSuffix(path, ".bin")
	bson, err := sent.MarshalBSON()
	if err != nil {
		fail("MarshalBSON failed: %v", err)
	}
	forms := []struct {
		ext  string
		data []byte
		read func(*Telemetry, []byte) error
	}{
		{".json", sent.ToJSON(), (*Telemetry).FromJSON},
		{".xml", sent.ToXML(), (*Telemetry).FromXML},
		{".bson", bson, (*Telemetry).UnmarshalBSON},
	}
	for _, form := range forms {
		if err := os.WriteFile(base+form.ext, form.data, 0o644); err != nil {
			fail("cannot write %s: %v", base+form.ext, err)
		}
		data, err := os.ReadFile(base + form.ext)
		if err != nil {
			fail("cannot open %s: %v", base+form.ext, err)
		}
		var copied Telemetry
		if err := form.read(&copied, data); err != nil || !copied.Equals(&sent) {
			fail("the %s copy is different: %v", form.ext, err)
		}
	}
	fmt.Printf("%s\n", sent.ToJSON())
}
