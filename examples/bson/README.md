# BSON example

Reads a `Telemetry` from a BSON document like one a MongoDB driver hands back,
writes it back in the canonical form every language writes, shows a large
`uint64_t` written as a Decimal128, and shows what the reader refuses. The same program in C++ ([main.cpp](main.cpp)), Python
([main.py](main.py)), Go ([main.go](main.go)) and Swift
([main.swift](main.swift)); all four write the same bytes, [expected.bson](expected.bson). The format is
defined in [BSON](../../docs/text-formats.md#bson); the messages are the ones
in [../README.md](../README.md).

## The input

[telemetry.bson](telemetry.bson), 224 bytes, is written by
[make_input.py](make_input.py) with Python's `struct`, independently of any
skywire code. It is what a driver might return for a stored `Telemetry`:

- `_id` (an ObjectId) and `received_at` (a UTC datetime), which are not
  members of `Telemetry` and are ignored;
- the members in another order than the spec's tags;
- numbers in the types the driver chose: `device_id`, a `uint32_t`, as an
  int32, and `battery_mv`, a `uint16_t`, as a double (3870.0). A reader takes
  any number type that holds the member's value exactly.

The canonical document, [expected.bson](expected.bson), is the same 186 bytes
as [../Telemetry.bson](../Telemetry.bson): the members in tag order, each in
the one type its member's type maps to (`device_id` an int64, `battery_mv` an
int32), and no `_id`. To store it in MongoDB, insert it as is (the server
adds an `_id`), or with your own `_id`.

## A large uint64_t

A `uint64_t` above 2^63-1 does not fit an int64, BSON's largest integer, so
`to_bson` writes it as a Decimal128 (type `0x13`): every tool then reads it as
the same positive number, never as a negative int64. The program sets
`timestamp_us` to 2^64-1, checks the document holds a Decimal128
`timestamp_us`, and reads it back.

## What is refused

Each of these is refused: `from_bson` returns false (in C++ and Python) and
`UnmarshalBSON` an error (in Go), leaving the message as it was, and
`init(bson:)` throws `SkywireText.Invalid` (in Swift):

| Document | Why |
|---|---|
| the input less its last byte | cut short |
| the input and one more byte | bytes after the document |
| `{status: int32 9}` | `status` is a 3-bit field |
| `{device_id: int64 -1}` | `device_id` is a `uint32_t` |
| `{timestamp_us: double 1.5}` | not a whole number |

## Running it

As for the [JSON example](../json/README.md), with `bson` for `json`:

```sh
c++ -std=c++17 -Iout -Ilang/cpp -Ilang/c -I<protozero>/include -o out/example_bson_cpp examples/bson/main.cpp
out/example_bson_cpp examples/bson/telemetry.bson out/out.bson
cp examples/bson/main.py py/
python3 py/main.py examples/bson/telemetry.bson out/out.bson
cp examples/bson/main.go go/
(cd go && go build -o example_bson_go .)
go/example_bson_go examples/bson/telemetry.bson out/out.bson
swiftc -o swift/example_bson_swift lang/swift/SWStream.swift lang/swift/SkywireText.swift \
    swift/Position.swift swift/Telemetry.swift examples/bson/main.swift
swift/example_bson_swift examples/bson/telemetry.bson out/out.bson
```

`ctest -R skywire_example:BSON` runs all four and checks each writes
`expected.bson`. To change the input, edit `make_input.py` and run
`python3 examples/bson/make_input.py examples/bson/telemetry.bson`.
