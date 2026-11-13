# XML example

Reads a `Telemetry` from XML written by another program, writes it back in the
canonical form every language writes, and shows what the reader refuses. The same program in C++ ([main.cpp](main.cpp)), Python
([main.py](main.py)), Go ([main.go](main.go)) and Swift
([main.swift](main.swift)); all four write the same bytes, [expected.xml](expected.xml). The format is defined
in [JSON, XML and BSON](../../docs/text-formats.md); the messages are the ones
in [../README.md](../README.md).

## The input

[telemetry.xml](telemetry.xml) uses what XML allows beyond the canonical form:

- an XML declaration, comments and indentation;
- `longitude` with spaces around the number, and `altitude_m` in a CDATA
  section;
- `callsign` in hexadecimal, `encoding="hex"`: it holds a byte that is not
  printable ASCII (BEL, `07`), which text cannot carry;
- `firmware`, which is not a member of `Telemetry` and is ignored, with an
  entity (`&amp;`) in it.

The canonical form, [expected.xml](expected.xml), has no declaration and no
whitespace, and writes `callsign` in hexadecimal again (on one line, wrapped
here):

    <Telemetry><device_id>4207</device_id><timestamp_us>1760959800123456</timestamp_us>
    <mode>5</mode><status>3</status><armed>1</armed><callsign encoding="hex">534b593107</callsign>
    <position><latitude>47.6062</latitude><longitude>-122.3321</longitude>
    <altitude_m>56.5</altitude_m></position><battery_mv>3870</battery_mv></Telemetry>

## What is refused

Each of these is refused: `from_xml` returns false (in C++ and Python) and
`FromXML` an error (in Go), leaving the message as it was, and `init(xml:)`
throws `SkywireText.Invalid` (in Swift):

| Input | Why |
|---|---|
| `<!DOCTYPE Telemetry [<!ENTITY x "y">]><Telemetry/>` | a document type declaration (and so any entity but the five predefined ones) |
| `<Position/>` | the root element is not `Telemetry` |
| `<Telemetry><mode>5</Telemetry>` | not well-formed |
| `<Telemetry><status>8</status></Telemetry>` | `status` is a 3-bit field |
| `<Telemetry><callsign encoding="base64">U0tZMQ==</callsign></Telemetry>` | the only encoding is `hex` |

## Running it

As for the [JSON example](../json/README.md), with `xml` for `json`:

```sh
c++ -std=c++17 -Iout -Ilang/cpp -Ilang/c -I<protozero>/include -o out/example_xml_cpp examples/xml/main.cpp
out/example_xml_cpp examples/xml/telemetry.xml out/out.xml
cp examples/xml/main.py py/
python3 py/main.py examples/xml/telemetry.xml out/out.xml
cp examples/xml/main.go go/
(cd go && go build -o example_xml_go .)
go/example_xml_go examples/xml/telemetry.xml out/out.xml
swiftc -o swift/example_xml_swift lang/swift/SWStream.swift lang/swift/SkywireText.swift \
    swift/Position.swift swift/Telemetry.swift examples/xml/main.swift
swift/example_xml_swift examples/xml/telemetry.xml out/out.xml
```

`ctest -R skywire_example:XML` runs all four and checks each writes
`expected.xml`.
