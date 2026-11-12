# Languages

| Language | `--language` | Runtime | Generated tests | Example |
|---|---|---|---|---|
| C | `c` | `cstream.h`, header-only | Google Test (C++) | `examples/c` |
| C++ | `c++`, `cpp` | `streamable.h`, `stream.h`, header-only; needs protozero | Google Test | `examples/cpp`, `examples/protobuf` |
| C# | `csharp`, `cs`, `c#` | none | a test program (`Main`) | `examples/csharp` |
| Go | `go`, `golang` | none | `go test` | `examples/go` |
| Java | `java` | none | a test program (`main`) | `examples/java` |
| JavaScript | `javascript`, `js` | none | `node:test` | `examples/javascript` |
| Python 3 | `python3`, `python` | none | `unittest` | `examples/python` |
| Rust | `rust` | none | `#[test]` | `examples/rust` |
| Swift | `swift` | `SWStream.swift` (and `SkywireText.swift` for JSON, XML and BSON), or the [Swift package](using.md#swift-package) | a test program (`@main`) | `examples/swift` |
| proto3 | `proto3` | | a text-format message | |

Every language puts the same bytes on the wire for the same specification.
[Generated code](generated-code.md) shows C and C++ and compares the in-place
functions across languages; the sections below cover Rust, C# and JavaScript,
the newest generators, in more detail.

## Rust

`--language rust` writes each message as a module of its own, named after the
class in snake_case: `NestyMcNestface` becomes `nesty_mc_nestface.rs`. The code
uses only `std`, so there is no runtime crate to add; it needs rustc 1.58 or
newer (edition 2021) and is rustfmt- and clippy-clean. Declare the module next
to the modules of the messages it contains, which it imports as
`super::<module>`:

```rust
mod nesty_inner;
mod nesty_mc_nestface;

use nesty_mc_nestface::NestyMcNestface;

fn main() -> std::io::Result<()> {
    let mut msg = NestyMcNestface::default();
    msg.set_a_myuint8_1(7);
    msg.b_first2_mut().set_a_myint16_1(-3);

    let wire = msg.serialize(); // NestyMcNestface::SIZE bytes
    let back = NestyMcNestface::from_bytes(&wire)?;
    assert_eq!(back, msg);
    Ok(())
}
```

| Item | Notes |
|---|---|
| `#[derive(Debug, Clone, Copy, Default, PartialEq)]` | `Default` is written out instead when a string is longer than 32 bytes, which std can't derive. |
| `SIZE`, `size()` | The wire size, a constant. |
| `serialize()`, `serialize_into(&mut Vec<u8>)` | `serialize_into` appends. |
| `write_to(&mut [u8])` | Writes into the first `SIZE` bytes of the slice, as nested messages do; panics if it is shorter. |
| `deserialize(&mut self, &[u8])`, `from_bytes(&[u8])` | Read from the start of the slice; extra bytes are ignored. A slice shorter than `SIZE` returns an `UnexpectedEof` `std::io::Error` and leaves the message unchanged. |
| `equals(&other)` | The same as `==`. |
| `x()`, `set_x(v)` | Fields are private. A bitfield setter keeps only the field's bits. |
| `x() -> &[u8; N]`, `set_x(&[u8])` | A string; the setter truncates or zero-pads to `N` bytes. |
| `x() -> &T`, `x_mut()`, `set_x(T)` | A nested message. |

A member named after a Rust keyword becomes a raw identifier (`r#type`, with
the setter `set_type`). wiregen rejects a member named `self`, `Self`,
`super`, `crate` or `_`, and one whose accessors would collide with the
generated items (`SIZE`, `size`, `serialize`, `serialize_into`, `write_to`,
`deserialize`, `from_bytes`, `equals`, `default`) or with another member's (`a` and `set_a`).
Names that aren't snake_case, and a class name that isn't CamelCase, are kept
as they are, with `#![allow(non_snake_case)]` or
`#![allow(non_camel_case_types)]` at the top of the module.

The specification's `namespace` and `includes` are not used. A nested message
is imported from its default module name, so generate it without overriding
its `outputimplfilename`. PZ mode is not available in Rust.

With `--generate_json`, `--generate_xml` or `--generate_bson` the struct also
has `to_json`/`from_json`, `to_xml`/`from_xml` or `to_bson`/`from_bson`, and
the module `skywire_text.rs` is written beside it, to be declared with the
others; no member's accessors can then take those names, and no message can
be `SkywireText` (see [Rust](text-formats.md#rust) in JSON, XML and BSON).

## C#

`--language csharp` (or `cs`, `c#`) writes each message as a
`public sealed class` in a `.cs` file of its own, in the specification's
`namespace` (or none, if it has none). The code is portable C# 7.3 with no
runtime library and no `unsafe` code, so it builds for .NET Framework 4.6.2
and later, Unity and .NET, with or without overflow checking.
Where the target has `Span<T>` (.NET Core 2.1+, .NET Standard 2.1+), the
generated helpers use it and `BinaryPrimitives` instead of writing one byte at
a time; define `SKYWIRE_PORTABLE` to build the portable code everywhere.

```csharp
using System;
using testspace;

public static class Program
{
    public static void Main()
    {
        NestyMcNestface msg = new NestyMcNestface();
        msg.a_myuint8_1 = 7;
        msg.b_first2.a_myint16_1 = -3;

        byte[] wire = msg.Serialize(); // msg.Size() bytes
        NestyMcNestface back = new NestyMcNestface();
        back.Deserialize(wire);
        Console.WriteLine(back.Equals(msg)); // True
    }
}
```

| Member | Notes |
|---|---|
| `Size()` | The wire size. |
| `Serialize()` | A new `byte[]` of `Size()` bytes. |
| `Serialize(byte[] buf, ref int offset)` | Writes at `offset` and advances it past the message, as nested messages do. |
| `Deserialize(byte[] data)`, `Deserialize(byte[] buf, ref int offset)` | Read from the start of `data`, or at `offset` (advancing it); extra bytes are ignored. With fewer than `Size()` bytes left, both overloads of `Serialize` and `Deserialize` throw `ArgumentException` and change nothing. |
| `x { get; set; }` | A property per member. A bitfield's setter keeps only the field's bits; a nested message's rejects `null`. |
| `Get_x()`, `Set_x(byte[])` | A string. `Get_x` returns a copy; `Set_x` copies, truncating or zero-padding to `N` bytes. |
| `Equals`, `GetHashCode`, `ToString` | The class is `IEquatable<X>`. `float` and `double` members compare by their bits, as they go on the wire, so NaN equals itself and -0.0 differs from 0.0. |

`char` and `int8_t` are `sbyte`; `unsigned char` and `uint8_t` are `byte`; the
other integer types are `short`, `ushort`, `int`, `uint`, `long` and `ulong`.
A member named after a C# keyword becomes a verbatim identifier (`@event`).
wiregen rejects a member whose name would collide with something the class
defines (`Size`, `Serialize`, `Deserialize`, `Equals`, `GetHashCode`,
`ToString`, `GetType`, `Put`, `Get`, the class's own name, ...) or with
another member's field or string methods (`a`'s field is `m_a`). The
specification's `includes` become `using` lines.

With `--generate_json`, `--generate_xml` or `--generate_bson` the class also
has `ToJson`/`FromJson`, `ToXml`/`FromXml` or `ToBson`/`FromBson`, and
`SkywireText.cs` is written beside it; a member cannot then take those
names or the helper types' (see [C#](text-formats.md#c_2) in JSON, XML and
BSON).

`examples/csharp` is a complete program, and `bench/` times C# like the other
languages. C# has no relay yet, so it is not in the ring test.
PZ mode is not available in C#.

## JavaScript

`--language javascript` (or `js`) writes each message as an ES module, `X.js`,
exporting `class X`, and its TypeScript declarations, `X.d.ts`. The module is
ES2020 and uses only `Uint8Array`, `DataView`, `BigInt` and `TextEncoder`, with
no Node APIs and no runtime package, so it runs unchanged in Node 18 and
later, current browsers, Deno and Bun, and through any bundler. TypeScript
5.1 or later picks up the declarations; nothing needs compiling.

```js
import { NestyMcNestface } from "./NestyMcNestface.js";

const msg = new NestyMcNestface();
msg.a_myuint8_1 = 7;
msg.b_first2.a_myint16_1 = -3; // a nested message's accessors
msg.c_myint64_3 = -9000000000n; // 64-bit integers are BigInts

const wire = msg.serialize(); // a Uint8Array of NestyMcNestface.SIZE bytes
const back = NestyMcNestface.fromBytes(wire);
console.log(back.equals(msg)); // true
```

| Member | Notes |
|---|---|
| `X.SIZE`, `size()` | The wire size; `SIZE` is read-only. |
| `serialize()` | A new `Uint8Array` of `SIZE` bytes. |
| `serializeInto(buf, offset = 0)` | Writes into a `DataView`, any typed array (a Node `Buffer` too) or an `ArrayBuffer`, at `offset`, and returns the offset after the message, as nested messages do. |
| `deserialize(bytes)`, `deserializeFrom(buf, offset = 0)`, `X.fromBytes(bytes)` | Read from the start of `bytes`, or at `offset` (returning the offset after the message); extra bytes are ignored. With fewer than `SIZE` bytes left, these and `serializeInto` throw `RangeError` and change nothing. |
| `x` | A getter and setter per member. Setters wrap to the member's type as typed arrays and C do (`int16_t` 40000 becomes -25536); a bitfield's keeps only the field's bits; a nested message's takes only that class. |
| `x`, `xText` | A string. The getter returns a copy of the `N` bytes as a `Uint8Array`; the setter takes a string (encoded as UTF-8) or bytes, truncated or zero-padded to `N`. `xText` decodes the bytes up to the first NUL as UTF-8. |
| `equals(other)`, `toString()`, `toJSON()` | `float` and `double` members compare with `Object.is`, so NaN equals itself and -0 differs from 0. `toJSON()` gives 64-bit integers as decimal strings and strings as arrays of bytes, so `JSON.stringify` works. |

`uint64_t` and `int64_t` are `bigint`; their setters also take an integral
`number`. Every other integer type and both floating-point types are
`number`; a `float` member holds its value rounded to single precision
(`Math.fround`), so it reads back exactly as it was sent. Member names may be
JavaScript reserved words (`msg.class`). wiregen rejects a member whose name
would collide with something the class defines (`size`, `serialize`,
`serializeInto`, `deserialize`, `deserializeFrom`, `fromBytes`, `equals`,
`toJSON`, `toString`, `constructor`, `SIZE`, `then`, `valueOf`, `__proto__`,
...) or with another member's backing property or text getter (`a`'s is
`_a`, string `s`'s is `sText`), and a class or nested type named after a
reserved word or a global the module uses (`Object`, `DataView`, ...). With
`--generate_json`, `--generate_xml` or `--generate_bson` the methods those
add are taken too (`toJsonText`, `readXml`, `fromBson`, ...), and a type
cannot be named `skywireText` (see [JavaScript](text-formats.md#javascript)
in JSON, XML and BSON). The
specification's `namespace` is not used; its `includes` become side-effect
imports (`import "./polyfill.js";`).

The `.js` names suit browsers, bundlers, Deno, TypeScript and Node 22 and
later, which detect ES module syntax. Node 18 and 20 need `"type": "module"`
in the nearest `package.json`. Inside a `"type": "commonjs"` package, set the
lang file's `outputimplfilename` to `{classname}.mjs` and
`outputheaderfilename` to `{classname}.d.mts`; nested messages are imported
by the same pattern.

NaN payloads: the ECMAScript specification leaves the bits of a NaN read or
written through a `DataView` up to the engine. V8 (Node, Chrome) keeps a quiet
NaN's payload, which `src/test/generated_tests/nan_payload.test.js` checks,
but a signaling `float` NaN comes back quiet, so a JavaScript relay can change
those bytes. `examples/javascript` is a complete program, `bench/` times
JavaScript like the other languages, and its relay is in the ring test. PZ
mode is not available in JavaScript.
