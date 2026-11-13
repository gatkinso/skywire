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

// The Go driver for skywire_text_diff: prints what the generated Go ring
// message makes of each input, as oracle.cpp does for C++. text_diff.cmake
// builds it in a module beside the generated package.
//
//	driver <inputs>
package main

import (
	"bufio"
	"encoding/binary"
	"encoding/hex"
	"os"

	ring "textdiff/generated"
)

func main() {
	raw, err := os.ReadFile(os.Args[1])
	if err != nil {
		panic(err)
	}
	out := bufio.NewWriter(os.Stdout)
	defer out.Flush()
	line := func(tag string, ok bool, data []byte) {
		if ok {
			out.WriteString(tag + " " + hex.EncodeToString(data) + "\n")
		} else {
			out.WriteString(tag + " -\n")
		}
	}
	for at := 0; at < len(raw); {
		n := int(binary.LittleEndian.Uint32(raw[at:]))
		data := raw[at+4 : at+4+n]
		at += 4 + n

		m := &ring.RingyMcRingface{}
		ok := m.FromJSON(data) == nil
		line("J", ok, m.ToJSON())
		m = &ring.RingyMcRingface{}
		ok = m.FromXML(data) == nil
		line("X", ok, m.ToXML())
		m = &ring.RingyMcRingface{}
		ok = m.UnmarshalBSON(data) == nil
		written, _ := m.MarshalBSON()
		line("B", ok, written)
	}
}
