# Examples

Small, complete programs that use the code wiregen generates, one for each
language, plus one that talks to protobuf. All of them use the same message,
`Telemetry`, from the same two specs, and ctest builds and runs every one.

| Example | Program | What it shows |
|---|---|---|
| C | [c/main.c](c/main.c) | Setters taking pointers, `Cstream`, `Telemetry_Equals` |
| C++ | [cpp/main.cpp](cpp/main.cpp) | `skyloupe::Stream`, `operator==` |
| Protobuf (C++ PZ mode) | [protobuf/main.cpp](protobuf/main.cpp) | A PZ-mode `Stream` talking to protoc's C++ classes, both ways |
| Swift | [swift/main.swift](swift/main.swift) | `SWStream`, `Equals` |
| Go | [go/main.go](go/main.go) | `Serialize`, `Deserialize` returning an `error` |
| Python | [python/main.py](python/main.py) | `serialize`, `deserialize`, `==` |
| Java | [java/Main.java](java/Main.java) | Messages in a package, unsigned values in wider types |
| Rust | [rust/main.rs](rust/main.rs) | Message modules, `from_bytes` returning `io::Result` |
| C# | [csharp/Program.cs](csharp/Program.cs) | Properties, a nested message's properties, `Equals` |
| JavaScript | [javascript/main.js](javascript/main.js) | Accessors, a `BigInt` member, `fromBytes`, `equals` |

Every one of those also writes and reads the message's JSON, XML and BSON.
Three more examples, in C++, Python, Go and Swift, are about those forms,
reading input written elsewhere:

| Example | Programs | What it shows |
|---|---|---|
| [JSON](json/README.md) | [main.cpp](json/main.cpp), [main.py](json/main.py), [main.go](json/main.go), [main.swift](json/main.swift) | JSON written by hand: any order and whitespace, a missing member, an unknown one; what is refused |
| [XML](xml/README.md) | [main.cpp](xml/main.cpp), [main.py](xml/main.py), [main.go](xml/main.go), [main.swift](xml/main.swift) | XML from another program: a declaration, comments, CDATA, a hexadecimal string; what is refused |
| [BSON](bson/README.md) | [main.cpp](bson/main.cpp), [main.py](bson/main.py), [main.go](bson/main.go), [main.swift](bson/main.swift) | A document as a MongoDB driver returns it: `_id`, other number types; a `uint64_t` as a Decimal128; what is refused |

Each directory has a README with the exact wiregen and compiler commands (run
from the repository root, writing into `out/`) and what the build needs:
include directories, what to compile and link, and the lang file.

## What the programs do

Every program except the protobuf one:

1. fills a `Telemetry`, including its nested `Position`, its bitfields and its
   string, with the values below;
2. serializes it and writes it to the file named on its command line;
3. reads the file back and deserializes a second message;
4. checks the two are equal, prints a few fields and the wire size, and exits
   non-zero if anything went wrong;
5. does the same with the message's JSON, XML and BSON forms (see
   [JSON, XML and BSON](../docs/text-formats.md)), written beside the file as
   `telemetry.json`, `telemetry.xml` and `telemetry.bson`, and prints the JSON.

The protobuf program fills the same message and sends it through a PZ-mode
stream to protoc's generated class for the same message, which checks it and
re-encodes it to the same bytes. It then changes two fields on the protobuf
side and decodes the result with the skywire class. See
[protobuf/README.md](protobuf/README.md), which also explains how to use a PZ
stream.

## The messages

[specs/Telemetry.json](specs/Telemetry.json) holds a
[specs/Position.json](specs/Position.json). Generate both into the same place;
the generated `Telemetry` imports `Position` itself. Between them they use both
byte orders, a bitfield unit, a fixed-length string and a nested message:

| Tag | Member | Type | Skywire wire format | Value set |
|---|---|---|---|---|
| 1 | `device_id` | `uint32_t` | 4 bytes, big-endian | 4207 |
| 2 | `timestamp_us` | `uint64_t` | 8 bytes, little-endian | 1760959800123456 |
| 3 | `mode` | `uint8_t`, 4 bits | one byte, `mode` in the low bits, | 5 |
| 4 | `status` | `uint8_t`, 3 bits | then `status`, | 11, stored as 3 |
| 5 | `armed` | `uint8_t`, 1 bit | then `armed` in the top bit | 1 |
| 6 | `callsign` | `string`, length 8 | 8 bytes, NUL-padded | `SKY1` |
| 7 | `position` | `Position` | 20 bytes: | |
| 7.1 | `latitude` | `double` | 8 bytes, big-endian | 47.6062 |
| 7.2 | `longitude` | `double` | 8 bytes, big-endian | -122.3321 |
| 7.3 | `altitude_m` | `float` | 4 bytes, little-endian | 56.5 |
| 8 | `battery_mv` | `uint16_t` | 2 bytes, little-endian | 3870 |

`status` is set to 11 to show a bitfield setter: 11 doesn't fit in 3 bits, so
the setter keeps the low bits, 3.

## The expected bytes

Every language writes the same 43 bytes, [Telemetry.bin](Telemetry.bin):

    00 00 10 6f                ← device_id
    40 a0 59 67 95 41 06 00    ← timestamp_us
    b5                         ← mode 5 | status 3 << 4 | armed 1 << 7
    53 4b 59 31 00 00 00 00    ← callsign "SKY1"
    40 47 cd 97 f6 2b 6a e8    ← position.latitude
    c0 5e 95 41 20 5b c0 1a    ← position.longitude
    00 00 62 42                ← position.altitude_m
    1e 0f                      ← battery_mv

Every language writes the same JSON, [Telemetry.json](Telemetry.json) (one
line, wrapped here):

    {"device_id":4207,"timestamp_us":"1760959800123456","mode":5,"status":3,"armed":1,
     "callsign":"SKY1","position":{"latitude":47.6062,"longitude":-122.3321,"altitude_m":56.5},
     "battery_mv":3870}

the same XML, [Telemetry.xml](Telemetry.xml), and the same 186-byte BSON
document, [Telemetry.bson](Telemetry.bson): `device_id` (a `uint32_t`) and
`timestamp_us` as int64, the bitfields and `battery_mv` as int32, `callsign`
as a string, and `position` as an embedded document of three doubles.

The protobuf example writes the same message as protobuf, 52 bytes,
[Telemetry.pb](Telemetry.pb). Each field is a tag byte (field number << 3 |
wire type) and its value. Protobuf leaves out zero fields and a string's NUL
padding, so the size depends on the values:

    08 ef 20                   ← 1 device_id, varint
    10 c0 c0 e6 ba d6 b2 90 03 ← 2 timestamp_us, varint
    18 05                      ← 3 mode
    20 03                      ← 4 status
    28 01                      ← 5 armed
    32 04 53 4b 59 31          ← 6 callsign, 4 bytes
    3a 17                      ← 7 position, a 23-byte message:
       09 e8 6a 2b f6 97 cd 47 40 ← 1 latitude, little-endian double
       11 1a c0 5b 20 41 95 5e c0 ← 2 longitude
       1d 00 00 62 42             ← 3 altitude_m, little-endian float
    40 9e 1e                   ← 8 battery_mv, varint

[protobuf/telemetry.txtpb](protobuf/telemetry.txtpb) is the same message as
protobuf text; `protoc --encode` turns it into `Telemetry.pb`.

## Include directories, runtimes and lang files

Each example's README explains its build. In short:

| Example | Include / import path | Compile and link with the messages | Lang file |
|---|---|---|---|
| C | generated dir, `lang/c` | nothing (header-only) | `lang/c/lang.c.json` |
| C++ | generated dir, `lang/cpp`, `lang/c`, protozero `include` | nothing (header-only) | `lang/cpp/lang.cpp.json` |
| Protobuf | as C++, plus protoc's output dir and protobuf `include` | protoc's `.pb.cc`, `libprotobuf` | `lang/cpp/lang.cpp.json`, `lang/proto/lang.proto3.json` |
| Swift | none | `lang/swift/SWStream.swift` and `SkywireText.swift`, same module | `lang/swift/lang.swift.json` |
| Go | none (one package dir) | the generated `skywire_text.go` (standard library only) | `lang/go/lang.go.json` |
| Python | generated dir on the import path | the generated `skywire_text.py` (standard library only) | `lang/python/lang.python3.json` |
| Java | `-cp` the class output dir | the generated `SkywireText.java` (JDK only) | `lang/java/lang.java.json` |
| Rust | none (`mod` next to the modules) | the generated `skywire_text.rs` (`std` only) | `lang/rust/lang.rust.json` |
| C# | none (`Example.csproj` next to the generated files) | the generated `SkywireText.cs` (base class library only) | `lang/csharp/lang.csharp.json` |
| JavaScript | ES module imports | the generated `skywire_text.js` | `lang/javascript/lang.javascript.json` |

The JSON, XML and BSON helpers are the ones `--generate_json`,
`--generate_xml` and `--generate_bson` need: header-only `text.h` and
`bson.h` (C++), `ctext.h` and `cbson.h` (C), `SkywireText.swift`, which ships
like `SWStream.swift`, and for the other languages one file wiregen writes
beside the messages.

The build copies every lang file next to `wiregen`, and `--language <name>`
reads it from there; `--lang_file <path>` uses another copy instead. The lang
file names the generated files and, for C, C++ and Swift, the runtime includes
each message starts with (`cstream.h`, `streamable.h`, `Foundation`).

## Tests

    ctest -R skywire_example

The examples are built and tested unless the project is configured with
`-DSKYWIRE_EXAMPLES=OFF`. The C, C++ and protobuf programs build with the
project (`example_c`, `example_cpp`, `example_protobuf`, and
`example_json_cpp`, `example_xml_cpp`, `example_bson_cpp`); `run_example.cmake`
generates and builds the others when their test runs.

| Test | Passes when |
|---|---|
| `skywire_example:C`, `:C++`, `:Swift`, `:Go`, `:Python`, `:Java`, `:Rust`, `:CSharp`, `:JavaScript` | The program succeeds and the files it wrote are `Telemetry.bin`, `Telemetry.json`, `Telemetry.xml` and `Telemetry.bson`, byte for byte. |
| `skywire_example:Protobuf` | protoc encodes `telemetry.txtpb` to `Telemetry.pb`, and the program succeeds and writes `Telemetry.pb`. |
| `skywire_example:JSON.C++`, `:JSON.Python`, `:JSON.Go`, `:JSON.Swift`, and likewise `XML` and `BSON` | The program reads its directory's input, refuses the bad inputs it tries, and writes the directory's `expected.*`, byte for byte. |

Like the round-trip tests, a test whose toolchain is missing reports Skipped.
The Go, Java, Rust and C# builds must also be clean: gofmt and `go vet`;
`javac -Xlint:all -Werror`; `rustc -D warnings`, plus rustfmt and clippy when
they are installed; C# with warnings as errors.

## Files

| Path | What it is |
|---|---|
| `specs/` | The two message specifications. |
| `<example>/` | The program and its README. |
| `Telemetry.bin` | The skywire wire bytes every language must write. |
| `Telemetry.json`, `Telemetry.xml`, `Telemetry.bson` | The JSON, XML and BSON every language must write. |
| `Telemetry.pb`, `protobuf/telemetry.txtpb` | The protobuf bytes, and the same message as text. |
| `CMakeLists.txt` | Builds the C, C++ and protobuf programs; defines the tests. |
| `run_example.cmake` | The test driver: builds the other languages, runs a program, compares its files. |
| `json/`, `xml/`, `bson/` | The JSON, XML and BSON examples: programs, input, expected output, README. |
| `run_format_example.cmake` | Their test driver. |

To change the message, edit the specs and the values in every program, then
regenerate the expected files: run any example to get `Telemetry.bin` and the
JSON, XML and BSON beside it, check their bytes by hand, and run
`protoc --encode=telemetry.pb.Telemetry Telemetry.proto < protobuf/telemetry.txtpb`
(on the `.proto` files the protobuf README generates) to get `Telemetry.pb`.
