# Why skywire?

skywire is for one job: you have a binary message format you don't control,
from a device, a radio, an ICD or another team's code, and you need to read
and write it, byte for byte, from more than one language.

## What it does well

- **One specification, nine languages, the same bytes.** C, C++, C#, Go,
  Java, JavaScript, Python, Rust and Swift from one JSON file. The tests pass
  one message through every language in turn and check every byte
  ([Testing](testing.md)).
- **The format is yours.** Byte order per member, bitfields with an explicit
  bit order, fixed-length strings, nested messages, and nothing on the wire
  that isn't a member: no tags, lengths or headers.
- **Plain generated code.** Structs or classes with getters and setters,
  readable and debuggable. Most languages need no runtime library; C and C++
  have header-only ones. The code is yours under any license
  ([License](license.md)).
- **Fast.** 8 to 45 nanoseconds per message in C, C++, Go, Rust, C#, Java
  and Swift on an M1 Pro ([Benchmarks](testing.md#benchmarks)).
- **Tests and relays generated too**, and every template is in a
  [lang file](lang-files.md) you can change to match your conventions.
- **A way into protobuf.** Generated C++ can also speak protobuf
  ([PZ mode](protobuf.md)), and wiregen writes the matching `.proto`.

## What it doesn't do

Be clear about these before you start:

- **Fixed layouts only.** No arrays or repeated fields, no length-prefixed or
  variable-length data, no optional or conditional members, no unions. Every
  message of a type is the same size.
- **No enums, checksums, framing or computed fields.** Members are plain
  numbers and bytes; validating them is up to you.
- **No schema evolution.** A change to the specification is a change to the
  format; there is no versioning or compatibility between versions of a
  message.
- **One message per file**, in JSON, which is wordier than a purpose-built
  language.
- **A small project**, with a small community.

If your format needs any of the first three, one of the tools below will
serve you better.

## Compared with

### Kaitai Struct

[Kaitai Struct](https://kaitai.io) is the closest alternative, and far more
expressive: its YAML `.ksy` files describe variable-length and conditional
data, repeats, switch-on-type, enums, computed values and seeking by offset,
and it has a web IDE and a large gallery of existing formats (file formats,
network protocols, game data). It generates parsers for many languages.

It is built mainly for reading. Writing (serialization) exists for only some
target languages and is less mature. skywire reads and writes
every language equally, and it generates plain structs with setters, not
parser objects. If you only need to parse, or your format has variable
structure, use Kaitai Struct.

### Protocol Buffers, FlatBuffers and Cap'n Proto

These own their wire format: you describe the data, and they decide the
bytes. That buys schema evolution (old and new readers interoperate), rich
types (repeated fields, maps, optional fields, unions), and large ecosystems
with RPC, reflection and tooling. FlatBuffers and Cap'n Proto also read data
in place without parsing it.

None of them can describe a format that already exists. If both ends of the
connection are yours, use one of them. If one end is a device or a document
you must match, they can't help; skywire can, and its C++ PZ mode lets the
same message also travel as protobuf.

### Hand-written packed structs

The traditional answer: a `#pragma pack` struct per message, with `htons` and
friends. It needs no tools and puts no limit on the layout.

It also leaves every detail to you: byte swapping on each field, alignment
and packing that differ between compilers, C bitfields whose layout the
standard leaves to the implementation, and a separate rewrite in every other
language, each with its own bugs. skywire generates the same thing with the
byte order, bit layout and tests done for you in every language.

### DFDL

[DFDL](https://daffodil.apache.org/docs/dfdl/) (Data Format Description
Language), an Open Grid Forum standard, annotates XML Schema to describe
nearly any text or binary format, and
[Apache Daffodil](https://daffodil.apache.org) parses and unparses with it.
It is more expressive than skywire and a standard, used where formats must
be documented formally.

It is also much heavier: XML Schema to write, a JVM to run, and the data comes
out as an infoset (XML or JSON) rather than as typed structs in your
language. (Daffodil can generate C for a subset of DFDL.) skywire's
specifications are a few lines of JSON, and its output is native code with
no runtime to deploy.

### Single-language libraries

Most languages have a library for declaring binary layouts:
[Construct](https://construct.readthedocs.io) for Python,
[binrw](https://binrw.rs) and [deku](https://github.com/sharksforarms/deku)
for Rust, `encoding/binary` for Go, and others. They are more flexible than
skywire and need no code generator, which makes them the better choice when
only one language is involved. skywire is for when the same message has to
be handled in several.
