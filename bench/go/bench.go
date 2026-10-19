// Times the generated Go code for the ring message. See ../README.md.
//
//	bench_go RingyMcRingface.bin
package main

import (
	"bytes"
	"fmt"
	"os"
	"testing"
)

const rounds = 5

var sink []byte

// best runs fn as a Go benchmark `rounds` times and returns the best time per
// operation in nanoseconds and the allocations per operation.
func best(fn func(b *testing.B)) (float64, int64) {
	bestNs := 0.0
	var allocs int64
	for r := 0; r < rounds; r++ {
		res := testing.Benchmark(func(b *testing.B) {
			b.ReportAllocs()
			fn(b)
		})
		ns := float64(res.T.Nanoseconds()) / float64(res.N)
		if r == 0 || ns < bestNs {
			bestNs = ns
		}
		allocs = res.AllocsPerOp()
	}
	return bestNs, allocs
}

func main() {
	testing.Init()
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "usage: bench_go RingyMcRingface.bin")
		os.Exit(2)
	}
	bin, err := os.ReadFile(os.Args[1])
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(2)
	}

	// The message must decode and re-encode to the same bytes.
	m := &RingyMcRingface{}
	if err := m.Deserialize(bin); err != nil {
		fmt.Fprintln(os.Stderr, "deserialize failed:", err)
		os.Exit(1)
	}
	if !bytes.Equal(m.Serialize(), bin) {
		fmt.Fprintln(os.Stderr, "serialize does not reproduce the input")
		os.Exit(1)
	}

	ser, serAllocs := best(func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			sink = m.Serialize()
		}
	})
	m2 := &RingyMcRingface{}
	de, deAllocs := best(func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = m2.Deserialize(bin)
		}
	})

	fmt.Printf("RESULT serialize %.1f\n", ser)
	fmt.Printf("RESULT deserialize %.1f\n", de)
	fmt.Printf("NOTE allocs/op: serialize %d, deserialize %d\n", serAllocs, deAllocs)
}
