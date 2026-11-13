# JSON example

Reads a `Telemetry` from JSON written by hand, writes it back in the canonical
form every language writes, and shows what the reader refuses. The same program in C++ ([main.cpp](main.cpp)), Python
([main.py](main.py)), Go ([main.go](main.go)) and Swift
([main.swift](main.swift)); all four write the same bytes, [expected.json](expected.json). The format is defined in
[JSON, XML and BSON](../../docs/text-formats.md); the messages are the ones
in [../README.md](../README.md).

## The input

[telemetry.json](telemetry.json) is not in the canonical form, and is still
a `Telemetry`:

- it is indented, and its members are in another order than the spec's tags;
- `battery_mv` is missing, so it reads as 0;
- `firmware` is not a member of `Telemetry`, so it is ignored;
- `timestamp_us`, a 64-bit integer, is a JSON string, as `to_json` writes
  it (a plain number is taken too).

The canonical form, [expected.json](expected.json), has no whitespace and the
members in tag order (on one line, wrapped here):

    {"device_id":4207,"timestamp_us":"1760959800123456","mode":5,"status":3,"armed":1,
     "callsign":"SKY1","position":{"latitude":47.6062,"longitude":-122.3321,"altitude_m":56.5},
     "battery_mv":0}

## What is refused

Each of these is refused: `from_json` returns false (in C++ and Python) and
`FromJSON` an error (in Go), leaving the message as it was, and
`init(json:)` throws `SkywireText.Invalid` (in Swift):

| Input | Why |
|---|---|
| `{"status": 9}` | `status` is a 3-bit field |
| `{"device_id": -1}` | `device_id` is a `uint32_t` |
| `{"timestamp_us": 1.5}` | not a whole number |
| `{"callsign": "TOO-LONG-CALLSIGN"}` | more than the 8 bytes `callsign` holds |
| `{"armed": true}` | not a number |
| `{"mode": 5,}` | not JSON (a trailing comma) |

## Running it

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
for spec in Position Telemetry; do
    $WIREGEN --input_file examples/specs/$spec.json --outdir out --language c++ --write_to_file \
        --generate_json --generate_xml --generate_bson
done
c++ -std=c++17 -Iout -Ilang/cpp -Ilang/c -I<protozero>/include -o out/example_json_cpp examples/json/main.cpp
out/example_json_cpp examples/json/telemetry.json out/out.json
```

In Python, Go and Swift, generate the messages with `--language python`,
`go` (with `--namespace main`, as each example is a command) or `swift`, and
the same three options, into a directory of their own (`py`, `go`, `swift`
here), then:

```sh
cp examples/json/main.py py/
python3 py/main.py examples/json/telemetry.json out/out.json

cp examples/json/main.go go/
printf 'module wiregen/example\n\ngo 1.21\n' > go/go.mod
(cd go && go build -o example_json_go .)
go/example_json_go examples/json/telemetry.json out/out.json

swiftc -o swift/example_json_swift lang/swift/SWStream.swift lang/swift/SkywireText.swift \
    swift/Position.swift swift/Telemetry.swift examples/json/main.swift
swift/example_json_swift examples/json/telemetry.json out/out.json
```

`ctest -R skywire_example:JSON` runs all four and checks each writes
`expected.json`.
