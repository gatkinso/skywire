# Fuzz targets

Generated deserializers read bytes from the network, and wiregen reads specs
and lang files. These targets feed both arbitrary input under
AddressSanitizer and UndefinedBehaviorSanitizer and check that nothing
crashes, leaks or misbehaves.

| Target | Input | Checks |
|---|---|---|
| `fuzz_c_sw` | bytes, to the generated C `_Deserialize` | Short input fails and consumes nothing; otherwise exactly `_Size()` bytes are consumed and `_Serialize` writes them back unchanged |
| `fuzz_cpp_sw` | bytes, to the generated C++ `deserialize` (SW) | The same, for C++ |
| `fuzz_c_cpp_diff` | bytes, to both | Both succeed or both fail; every member equal (floats bit for bit); both serialize to the same bytes |
| `fuzz_cpp_pz` | bytes, to the generated C++ protobuf decoder (PZ) | Bad input fails and leaves the stream alone; a decoded message re-encodes canonically and round-trips through SW |
| `fuzz_cpp_json` | text, to the generated C++ `from_json` | Bad input fails and leaves the message alone; a decoded message's JSON and BSON are canonical (read back to the same JSON and BSON), and its XML and SW forms carry it unchanged |
| `fuzz_cpp_xml` | text, to the generated C++ `from_xml` | The same, for XML |
| `fuzz_cpp_bson` | bytes, to the generated C++ `from_bson` | The same, for BSON |
| `fuzz_c_cpp_text` | bytes, as JSON, XML and BSON, to the generated C and C++ | For each form: C accepts exactly what C++ does; then C writes back exactly C++'s bytes; when it refuses, the C message is unchanged |
| `fuzz_spec` | a message spec's JSON | wiregen generates it (code and tests) for every language without crashing, and no generator throws with the shipped lang files |
| `fuzz_lang` | a lang file's JSON | wiregen generates the ring spec with it without crashing; a bad lang file is reported, not fatal |

The message is the ring test's `RingyMcRingface`
([`../generated_tests/ring`](../generated_tests/ring/README.md)): every native
type, both byte orders, strings, a nested message, and signed and unsigned
bitfields packed lsb and msb first. Its bitfield units are full, so every input
bit survives a round trip.

## In ctest

Every build with tests builds these targets as plain programs
(`replay_main.cpp`) that run each seed once, as `skywire_fuzz:<target>`. That
is how fixed crashes stay fixed on every platform, MSVC included.

## Fuzzing

libFuzzer needs Clang; on macOS, Homebrew's (`brew install llvm`), not
Apple's. Use a separate build directory:

```
cmake -S . -B build-fuzz -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DSKYWIRE_FUZZ=ON -DSKYWIRE_EXAMPLES=OFF \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build-fuzz --target fuzzers
scripts/run_fuzzers.py build-fuzz                  # every target, 60 s each
scripts/run_fuzzers.py build-fuzz --seconds 1800 fuzz_spec
```

`SKYWIRE_FUZZ` instruments the whole build with ASan, UBSan and coverage. The
`skywire_fuzz:*` ctest tests then replay the seeds through libFuzzer.
`run_fuzzers.py` keeps each target's growing corpus in `build-fuzz/fuzz/work/`
between runs, and puts any failing input in `build-fuzz/fuzz/artifacts/`.

A single target can also be run by hand, as any libFuzzer program:
`build-fuzz/src/test/fuzz/fuzz_cpp_pz -max_total_time=60 corpus_dir/`.

## When a fuzzer finds something

1. Reproduce it: `build-fuzz/src/test/fuzz/<target> <artifact>`.
2. Fix the bug.
3. Copy the input into `corpus/<target>/` with a name that says what it
   is, so ctest replays it from then on.

## Seeds

- The SW targets: `RingyMcRingface.bin` and [`corpus/sw/`](corpus/sw), inputs
  one byte short, one byte long, empty, and all ones (NaNs, negative
  bitfields).
- `fuzz_cpp_pz`: the ring message and `examples/Telemetry.pb` encoded as
  protobuf, and hand-made damage in [`corpus/fuzz_cpp_pz/`](corpus/fuzz_cpp_pz): a
  truncated varint, a bad length, a wrong wire type, an oversized string,
  unknown and repeated fields, an overlong varint.
- `fuzz_cpp_json`, `fuzz_cpp_xml`: the ring message as JSON and XML
  ([`corpus/text/`](corpus/text)), and in
  [`corpus/fuzz_cpp_json/`](corpus/fuzz_cpp_json) and
  [`corpus/fuzz_cpp_xml/`](corpus/fuzz_cpp_xml) escapes, NaN and the
  infinities, hex strings, CDATA, a DOCTYPE, repeated and unknown members,
  deep nesting, bad UTF-8 and out-of-range values.
- `fuzz_cpp_bson`: the ring message as BSON ([`corpus/text/`](corpus/text)),
  and in [`corpus/fuzz_cpp_bson/`](corpus/fuzz_cpp_bson) uint64 values as
  Decimal128, a binary string, NaN, Decimal128 with exponents, a negative
  int64 for a `uint64_t`, unknown elements of many types, a truncated document
  and deep nesting.
- `fuzz_c_cpp_text`: all of the above text and BSON seeds.
- `fuzz_spec`: every spec in the tree (`src/test/generated_tests`,
  `examples/specs`, `cfg`).
- `fuzz_lang`: every lang file in `lang/`.

[`json.dict`](json.dict) gives the two wiregen targets and `fuzz_cpp_json`
JSON syntax and the spec and lang-file keywords; [`xml.dict`](xml.dict) gives
`fuzz_cpp_xml` and `fuzz_c_cpp_text` XML syntax and the ring message's
element names.

## CI

The `Fuzz` workflow (`.github/workflows/fuzz.yml`) runs only by hand, from the
Actions tab or with `gh workflow run fuzz.yml -f seconds=600` (seconds per
target); it uploads the failing inputs.
