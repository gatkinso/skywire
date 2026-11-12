# JSON, XML and BSON

Besides the binary wire format, a message can be written as, and read from,
JSON or XML text, for logs, test fixtures, configuration, or a web service,
and BSON, for MongoDB and other BSON stores. Each is asked for separately:

    wiregen --language c++ --generate_json --generate_xml --generate_bson --input_file Telemetry.json --outdir out --write_to_file

`--generate_json` adds `to_json` and `from_json`, `--generate_xml` adds
`to_xml` and `from_xml`, `--generate_bson` adds `to_bson` and `from_bson`
(in C, `_ToJson`, `_FromJson` and so on; see [C](#c)). Without them nothing
changes: the generated code is exactly what it was. With `--generate_tests`
the generated tests also check these forms.

Every language has them; proto3, a schema with no code to add them to,
refuses the options. Every language writes the same text and the same BSON
for the same message, byte for byte, as they already put the same bytes on
the wire, so this page is their specification.

## JSON

`examples/specs/Telemetry.json` holding some values, as `to_json()` writes it
(on one line, wrapped here):

```json
{"device_id":42,"timestamp_us":"1700000000000000","mode":3,"status":1,"armed":1,"callsign":"N123AB",
 "position":{"latitude":47.5,"longitude":-122.25,"altitude_m":120.5},"battery_mv":3700}
```

An object, with one member per message member, named as in the
specification, in tag order (the wire order), with no whitespace.

| Member | In JSON |
|---|---|
| 8-, 16- and 32-bit integers, bitfields | A number. A `char` is written as signed (-128 to 127), whatever its signedness in the language. |
| `int64_t`, `uint64_t` | A string holding the decimal number (`"-9000000000"`), as in protobuf's JSON, since many JSON readers keep numbers as doubles. |
| `float`, `double` | A number, as below. NaN and the infinities are the strings `"NaN"`, `"Infinity"` and `"-Infinity"`. |
| `string` (`char[N]`) | A string of its bytes up to the trailing NULs. Bytes 0x20 to 0x7E are themselves, with `"` and `\` escaped as `\"` and `\\`; every other byte is `\u00xx` (lower-case hex). Each byte is one character, U+0000 to U+00FF. |
| a nested message | An object, the same way. |

### Floating point

A `float` or `double` is written with the fewest significant digits that read
back as exactly the same value of that type, so a `float` holding 0.1 is
`0.1`, not `0.10000000149011612`. If two such digit strings are equally near
the value, the one ending in an even digit is used. The digits are then laid
out as JavaScript's `Number.prototype.toString` lays them out:

| Value | Text |
|---|---|
| 3.5 | `3.5` |
| 100 | `100` |
| 1.5 × 10⁻⁶ | `0.0000015` |
| 10⁻⁷ | `1e-7` |
| 10²¹ | `1e+21` |
| −0 | `-0` (unlike JavaScript, the sign is kept) |

The decimal point is always `.`, whatever the program's locale.

## XML

The same message, as `to_xml()` writes it (wrapped here):

```xml
<Telemetry><device_id>42</device_id><timestamp_us>1700000000000000</timestamp_us><mode>3</mode>
<status>1</status><armed>1</armed><callsign>N123AB</callsign><position><latitude>47.5</latitude>
<longitude>-122.25</longitude><altitude_m>120.5</altitude_m></position><battery_mv>3700</battery_mv></Telemetry>
```

The root element is named after the message's class, with one child element
per member, named after the member, in tag order. A nested message is an
element named after the member, holding the nested message's members. There
is no XML declaration and no whitespace between elements.

| Member | In XML |
|---|---|
| Numbers | The same text as in JSON, but 64-bit integers, `NaN` and the infinities are not quoted. |
| `string` | Its bytes up to the trailing NULs, with `<`, `>`, `&`, `"` and `'` as `&lt;`, `&gt;`, `&amp;`, `&quot;` and `&apos;`. XML cannot hold most control characters, so if any byte is outside 0x20 to 0x7E, the element instead holds every byte in hex and says so: `<callsign encoding="hex">4e31320001</callsign>`. |

## BSON

BSON is MongoDB's binary JSON ([bsonspec.org](https://bsonspec.org), version
1.1). The same message, as `to_bson()` writes it, shown as its elements:

```text
188-byte document
  device_id     int64   42
  timestamp_us  int64   1700000000000000
  mode          int32   3
  status        int32   1
  armed         int32   1
  callsign      string  "N123AB"
  position      document
    latitude    double  47.5
    longitude   double  -122.25
    altitude_m  double  120.5
  battery_mv    int32   3700
```

A document, with one element per member, named as in the specification, in
tag order. Every number is little-endian, as BSON requires, whatever the
member's wire byte order.

| Member | In BSON |
|---|---|
| `char`, `int8_t`, `uint8_t`, `unsigned char`, `int16_t`, `uint16_t`, `int32_t`, and bitfields of those types | int32. A `char` is written as signed, as in JSON. |
| `uint32_t`, `int64_t`, and bitfields of those types | int64, since int32 cannot hold every `uint32_t`. |
| `uint64_t` and its bitfields | int64 up to 2^63-1. A larger value is a Decimal128 holding the integer (exponent 0), so no BSON reader sees it as negative. |
| `float`, `double` | double. A `float` is widened exactly. Every NaN is written as the same quiet NaN (`0x7FF8000000000000`), so every language writes the same bytes; -0 keeps its sign. |
| `string` (`char[N]`) | Its bytes up to the trailing NULs: a string if they are UTF-8 holding no NUL, otherwise binary (subtype 0). |
| a nested message | An embedded document, the same way. |

Most readers, MongoDB's tools included, see the values as written: a
`uint64_t` of 2^64-1 is the Decimal128 `18446744073709551615`, not -1. The
generated tests check the exact bytes; `skywire_generated_tests:BsonInterop`
checks them against pymongo's BSON library, both ways.

## Reading

`from_json`, `from_xml` and `from_bson` read these forms, and more besides,
so what is written by hand or by another JSON, XML or BSON library is
accepted:

- Whitespace is allowed anywhere JSON or XML allows it, and members may come
  in any order.
- A missing member reads as zero (an empty string, an all-zero nested
  message). `{}` and `<Telemetry/>` are the all-zero message.
- Unknown members are ignored, whatever they hold. If a member appears more
  than once, the last one counts.
- An integer member takes any JSON number that is a whole number in its
  type's range (`100`, `1.0e2`), or a string holding one (`"100"`), as
  64-bit members are written. A bitfield must fit its width: a 3-bit unsigned
  field takes 0 to 7, a 3-bit signed one -4 to 3. A `char` takes -128 to 255.
- A `float` or `double` takes any JSON number, rounded to the type, or a
  string holding one, or `"NaN"`, `"Infinity"` or `"-Infinity"`. A number too
  large for the type (`1e39` for a `float`) is an error.
- A `string` member takes a string of at most its length in characters, each
  U+0000 to U+00FF (one byte each), whether escaped (`é`) or not (`é`);
  a shorter one is zero-padded. In XML, `encoding="hex"` gives the bytes in
  hex, in either case.
- In BSON an integer member takes an int32, int64, Decimal128 or double, and
  a `float` or `double` an int32, int64 or double, under the same rules: a
  whole number in range for an integer (a negative one is never read into a
  `uint64_t` as its bits), and a number too large for a `float` fails. A
  `string` takes a string or binary of subtype 0. Unknown elements of any
  type in the specification are skipped.
- In XML the root element must be named after the class. Comments,
  processing instructions, CDATA sections, the five predefined entities and
  character references are understood; namespaces are not, and attributes
  other than `encoding` are ignored. Surrounding whitespace in a number is
  ignored, but not in a string.

Anything else fails: text that is not JSON or not well-formed XML, a number
out of range or not whole, a string too long, a value of the wrong kind
(`true`, `null`, an array, an element holding elements where a number
belongs, text where a nested message belongs), text that is not UTF-8. In
BSON: a document whose length is not the bytes it has, bytes after it, a
missing terminator, a string or binary whose length runs past its document, a
string without its NUL or not UTF-8, an element type BSON does not define
(its length is unknown), a boolean other than 0 or 1, and a Decimal128 that is
infinite, NaN, not canonical or not whole, read into an integer. **On failure
the message is left unchanged.**

For safety, nesting deeper than 128 levels (counting unknown members) fails,
and an XML document type declaration (`<!DOCTYPE`) fails, so there are no
external or user-defined entities to expand.

## C++

| Method | |
|---|---|
| `std::string to_json() const` | The JSON text. |
| `bool from_json(std::string_view text)` | Reads JSON text; false on failure, leaving the message unchanged. |
| `std::string to_xml() const` | The XML text. |
| `bool from_xml(std::string_view text)` | Reads XML text; false on failure, leaving the message unchanged. |
| `std::string to_bson() const` | The BSON document's bytes. |
| `bool from_bson(std::string_view bytes)` | Reads one BSON document, exactly `bytes`; false on failure, leaving the message unchanged. |

```cpp
telemetry::Telemetry msg;
msg.Set_device_id(42);
std::string json = msg.to_json();

telemetry::Telemetry copy;
if (false == copy.from_json(json)) { /* not a Telemetry */ }
```

The runtime is the header-only `lang/cpp/text.h` and `lang/cpp/bson.h`
(installed with `stream.h`), which use only the standard library; they are
included only when asked for. A message's
`to_json(skyloupe::text::JsonWriter&)`,
`from_json(const skyloupe::text::JsonValue&)`,
`to_xml(skyloupe::text::XmlWriter&, std::string_view element)`,
`from_xml(const skyloupe::text::XmlNode&)`,
`to_bson(skyloupe::bson::Writer&)` and
`from_bson(const skyloupe::bson::Document&)` are what nested messages use, and
can put a message inside a larger document of your own:

```cpp
skyloupe::bson::Writer w;
w.begin_document();
w.value("received_at", int64_t(1700000000));
w.document("telemetry");
msg.to_bson(w);
w.end_document();
std::string record = w.take();
```

A member cannot be named `to_json` or `from_json` with `--generate_json`,
`to_xml` or `from_xml` with `--generate_xml`, or `to_bson` or `from_bson`
with `--generate_bson`.

## C

`--language c` generates functions named for the message, as for the wire
format:

| Function | |
|---|---|
| `size_t X_ToJson(const X* m, char* buf, size_t cap)` | The JSON text, NUL-terminated, into `buf`; returns its length. |
| `int X_FromJson(X* m, const char* text, size_t len)` | Reads `len` bytes of JSON text; 0, or -1 leaving the message unchanged. |
| `size_t X_ToXml(const X* m, char* buf, size_t cap)` | The XML text, as `_ToJson`. |
| `int X_FromXml(X* m, const char* text, size_t len)` | Reads XML text, as `_FromJson`. |
| `size_t X_ToBson(const X* m, uint8_t* buf, size_t cap)` | The BSON document into `buf`; returns its length. |
| `int X_FromBson(X* m, const uint8_t* bytes, size_t len)` | Reads one BSON document, exactly `len` bytes; 0, or -1 leaving the message unchanged. |

`_ToJson` and `_ToXml` follow `snprintf`: they always return the whole
text's length (without the NUL), write at most `cap` bytes, and end what they
write with a NUL, so a call with a NULL buffer gives the size to allocate.
`_ToBson` returns the document's length too; the document is complete only
when that is no more than `cap`.

```c
Telemetry msg;
memset(&msg, 0, sizeof(msg));
msg.device_id = 42;

size_t n = Telemetry_ToJson(&msg, NULL, 0);
char* json = malloc(n + 1);
Telemetry_ToJson(&msg, json, n + 1);

Telemetry copy;
if (0 != Telemetry_FromJson(&copy, json, n)) { /* not a Telemetry */ }
```

The runtime is the header-only `lang/c/ctext.h` and `lang/c/cbson.h`
(installed with `cstream.h`), C99 with only the standard library, and
included only when asked for. They write and read exactly what the C++ runtime
does: the generated tests check both against the same expected bytes, and
the `fuzz_c_cpp_text` fuzz target checks that C accepts exactly the input C++
does and writes back exactly the same bytes. Reading streams through the input
once without building a tree; BSON is checked where it lies and needs no
allocation, and JSON and XML allocate only for a number written in more than
about 100 bytes. Neither depends on the C locale.

`X_WriteJson(const X*, sw_json_writer*)`, `X_ReadJson(X*, sw_json*)`,
`X_WriteXml(const X*, sw_out*, const char* element)`,
`X_ReadXml(X*, sw_xml*, const sw_xml_tag*)`, `X_WriteBson(const X*, sw_out*)`
and `X_ReadBson(X*, const sw_bson_doc*)` are what nested messages use, and can
put a message inside a larger document of your own:

```c
uint8_t record[512];
sw_out o;
sw_out_init(&o, record, sizeof(record));
size_t start = sw_bson_begin(&o);
sw_bson_put_int64_t(&o, "received_at", 1700000000);
sw_bson_document(&o, "telemetry");
Telemetry_WriteBson(&msg, &o);
sw_bson_end(&o, start);
/* o.len bytes; complete if o.len <= sizeof(record) */
```

No member name is reserved in C.

## Python

| Method | |
|---|---|
| `to_json()` | The JSON text, a `str`. |
| `from_json(text)` | Reads JSON from a `str` or UTF-8 `bytes`; `False` on failure, leaving the message unchanged. |
| `to_xml()` | The XML text, a `str`. |
| `from_xml(text)` | Reads XML, as `from_json`. |
| `to_bson()` | The BSON document, `bytes`. |
| `from_bson(data)` | Reads one BSON document, exactly `data`; `False` on failure, leaving the message unchanged. |

```python
from Telemetry import Telemetry

msg = Telemetry()
msg.Set_device_id(42)
text = msg.to_json()

copy = Telemetry()
if not copy.from_json(text):
    ...  # not a Telemetry
```

The helpers are in `skywire_text.py`, which wiregen writes beside the
generated modules when any of the options is given (standard library only).
They write and read exactly what the C++ runtime does: the generated tests
check the same expected bytes, and `skywire_text_diff:python3` checks that
Python accepts exactly the input C++ does and writes back the same bytes. A
`float` member is written as the 32-bit float it is on the wire, and read by
rounding the decimal straight to 32 bits. A value a member cannot hold (an
integer out of its type's range, a float beyond 32 bits) raises an error
when written, as `serialize` does.

`_write_json(w)`, `_read_json(value)`, `_write_xml(w, element)`,
`_read_xml(node)`, `_write_bson(w)` and `_read_bson(elements)` are what nested
messages use. A member cannot be named `to_json`, `from_json`, `_write_json`
or `_read_json` with `--generate_json`, nor the same for `xml` or `bson` with
those options.

## Go

| Method | |
|---|---|
| `ToJSON() []byte` | The JSON text. |
| `FromJSON(b []byte) error` | Reads JSON (UTF-8); on an error the message is unchanged. |
| `ToXML() []byte` | The XML text. |
| `FromXML(b []byte) error` | Reads XML, as `FromJSON`. |
| `MarshalBSON() ([]byte, error)` | The BSON document; the error is always nil. |
| `UnmarshalBSON(b []byte) error` | Reads one BSON document, exactly `b`; on an error the message is unchanged. |

```go
msg := &telemetry.Telemetry{}
msg.Set_device_id(42)
text := msg.ToJSON()

var copy telemetry.Telemetry
if err := copy.FromJSON(text); err != nil {
	// not a Telemetry
}
```

The helpers are in `skywire_text.go`, which wiregen writes into the
message's package when any of the options is given (standard library only;
every name in it starts with `sw` and is unexported). They write and read
exactly what the C++ runtime does: the generated tests check the same
expected bytes, and `skywire_text_diff:go` checks that Go accepts exactly the
input C++ does and writes back the same bytes. A `float32` member is read by
rounding the decimal straight to 32 bits.

The messages also work with the standard library and the MongoDB driver:

- `MarshalJSON` and `UnmarshalJSON` make a message a `json.Marshaler` and
  `json.Unmarshaler`. `UnmarshalJSON` is `FromJSON`. `json.Marshal` keeps the
  canonical form but escapes `<`, `>` and `&` in strings, as `\u003c`,
  `\u003e` and `\u0026` (still valid JSON that reads back the same); use
  `ToJSON`, or an `Encoder` with `SetEscapeHTML(false)`, for the exact bytes.
- `MarshalXML` and `UnmarshalXML` make it an `xml.Marshaler` and
  `xml.Unmarshaler`. The element takes the name `encoding/xml` gives it, its
  text is escaped the way `encoding/xml` escapes, and `UnmarshalXML` takes
  the element as `encoding/xml` parses it, then reads its members as
  `FromXML` does. `ToXML` and `FromXML` are the canonical form.
- `MarshalBSON` and `UnmarshalBSON` are the MongoDB Go driver's
  `bson.Marshaler` and `bson.Unmarshaler`, so a message can be stored and
  read as a document as it is.

`writeJSON`, `readJSON`, `writeXML`, `readXML`, `writeBSON` and `readBSON`
are what nested messages use. With `--generate_json` a member cannot be
named `ToJSON`, `FromJSON`, `MarshalJSON`, `UnmarshalJSON`, `writeJSON` or
`readJSON`; likewise for XML with `--generate_xml`, and `MarshalBSON`,
`UnmarshalBSON`, `writeBSON` and `readBSON` with `--generate_bson`.

## C#

| Method | |
|---|---|
| `string ToJson()` | The JSON text. |
| `bool FromJson(string json)`, `bool FromJson(byte[] utf8)` | Reads JSON; false, with the message unchanged, if it is not the JSON form of one. A string with a lone surrogate, like bytes that are not UTF-8, is refused. |
| `string ToXml()` | The XML text. |
| `bool FromXml(string xml)`, `bool FromXml(byte[] utf8)` | Reads XML, as `FromJson`. |
| `byte[] ToBson()` | The BSON document. |
| `bool FromBson(byte[] bson)` | Reads one BSON document, exactly `bson`, as `FromJson`. |

```csharp
Telemetry msg = new Telemetry();
msg.device_id = 42;
string text = msg.ToJson();

Telemetry copy = new Telemetry();
if (!copy.FromJson(text))
{
    // not a Telemetry
}
```

The helpers are in `SkywireText.cs`, which wiregen writes beside the classes
when any of the options is given. It is C# 7.3 with only the base class
library (`System.Numerics` for `BigInteger`, which an old-style .NET
Framework project must reference), so it builds wherever the generated
classes do: .NET Framework 4.6.2 and later, Unity and .NET. Its
types are `internal`, in the namespace `Skywire.Text`, and every one starts
with `Sw`; the file is the same for every message, so build one copy into
each assembly that holds generated classes. The generated tests check the
same expected bytes as every other language, and `skywire_text_diff:csharp`
checks that C# accepts exactly the input C++ does and writes back the same
bytes. Numbers are read and written with exact arithmetic rather than
`double.Parse` and `ToString`, which differ between .NET Framework and .NET;
a `float` member is read by rounding the decimal straight to 32 bits.

`WriteJson`, `ReadJson`, `WriteXml`, `ReadXml`, `WriteBson` and `ReadBson`
are what nested messages use. With `--generate_json` a member cannot be
named `ToJson`, `FromJson`, `WriteJson` or `ReadJson`; likewise for XML with
`--generate_xml` and BSON with `--generate_bson`. With any of them a member
cannot take the name of one of the helper types: `SwText`,
`SwInvalidException`, `SwJson`, `SwJsonMember`, `SwJsonWriter`, `SwXml`,
`SwXmlWriter`, `SwBson`, `SwBsonElement` or `SwBsonWriter`.

## Rust

| Function | |
|---|---|
| `to_json(&self) -> String` | The JSON text. |
| `from_json(text: impl AsRef<[u8]>) -> std::io::Result<Self>` | Reads JSON from a `&str`, a `String` or bytes; an `InvalidData` error if it is not the JSON form of one (bytes that are not UTF-8 included). |
| `to_xml(&self) -> String` | The XML text. |
| `from_xml(text: impl AsRef<[u8]>) -> std::io::Result<Self>` | Reads XML, as `from_json`. |
| `to_bson(&self) -> Vec<u8>` | The BSON document. |
| `from_bson(bson: impl AsRef<[u8]>) -> std::io::Result<Self>` | Reads one BSON document, exactly `bson`, as `from_json`. |

```rust
mod skywire_text;
mod telemetry;

use telemetry::Telemetry;

fn main() -> std::io::Result<()> {
    let mut msg = Telemetry::default();
    msg.set_device_id(42);
    let text = msg.to_json();

    let copy = Telemetry::from_json(&text)?;
    assert_eq!(copy, msg);
    Ok(())
}
```

The readers build a new message, as `from_bytes` does, so there is nothing
to leave unchanged on an error. The helpers are the module `skywire_text`
(`skywire_text.rs`), which wiregen writes beside the message modules when
any of the options is given; declare it next to them, as they import it as
`super::skywire_text`. It uses only `std`, builds with rustc 1.58 and later,
and is rustfmt- and clippy-clean; its items are `pub(crate)` and it allows
`dead_code`, as a crate may use only some of the forms. The file is the
same for every message. Floats are written from the exact decimal value std
prints, rounded ties to even here, so the text does not depend on the
rustc version's own rounding; `str::parse` reads them, a `float` member
straight to 32 bits. The generated tests check the same expected bytes as
every other language, and `skywire_text_diff:rust` checks that Rust accepts
exactly the input C++ does and writes back the same bytes.

`write_json`, `read_json`, `write_xml`, `read_xml`, `write_bson` and
`read_bson` (`pub(crate)`) are what nested messages use. With
`--generate_json` a member cannot be named `to_json`, `from_json`,
`write_json` or `read_json`, as its getter would be; likewise for XML with `--generate_xml` and BSON with
`--generate_bson`. With any of them a message or nested type cannot be named
so that its module would be `skywire_text` (`SkywireText`).

## Java

| Method | |
|---|---|
| `String toJson()` | The JSON text. |
| `static X fromJson(String json)`, `static X fromJson(byte[] utf8)` | Reads JSON; throws `IllegalArgumentException` if it is not the JSON form of an `X`. A string with a lone surrogate, like bytes that are not UTF-8, is refused. |
| `String toXml()` | The XML text. |
| `static X fromXml(String xml)`, `static X fromXml(byte[] utf8)` | Reads XML, as `fromJson`. |
| `byte[] toBson()` | The BSON document. |
| `static X fromBson(byte[] bson)` | Reads one BSON document, exactly `bson`, as `fromJson`. |

```java
Telemetry msg = new Telemetry();
msg.Set_device_id(42);
String text = msg.toJson();

try {
    Telemetry copy = Telemetry.fromJson(text);
} catch (IllegalArgumentException e) {
    // not a Telemetry
}
```

The readers build a new message, so there is nothing to leave unchanged on
an error. The helpers are the class
`skywire.text.SkywireText`, in `SkywireText.java`, which wiregen writes
beside the classes when any of the options is given; it goes in the
package `skywire.text` (in a Maven or Gradle layout,
`skywire/text/SkywireText.java`), and the classes import it. It is Java 8
with only the standard library, and the same for every message. Floats are
written from their exact decimal value (`BigDecimal`), rounded ties to even
here, and read with `Double.parseDouble`, or `Float.parseFloat` for a
`float` member, so the decimal is rounded straight to 32 bits. The
generated tests check the same expected bytes as every other language, and
`skywire_text_diff:java` checks that Java accepts exactly the input C++
does and writes back the same bytes.

`writeJson`, `readJson`, `writeXml`, `readXml`, `writeBson` and `readBson`
are what nested messages use; they are public, as a nested message can be
in another package. Members are fields, which never clash with methods in
Java, so the options reserve no member names but one: with any of them
neither the class nor a member can be named `SkywireText`, which would hide
the helper class.

## Swift

| Method | |
|---|---|
| `toJson() -> String` | The JSON text. |
| `init(json: String) throws`, `init(json: [UInt8]) throws` | Reads JSON; throws `SkywireText.Invalid` if it is not the JSON form of one (bytes that are not UTF-8 included). |
| `toXml() -> String` | The XML text. |
| `init(xml: String) throws`, `init(xml: [UInt8]) throws` | Reads XML, as `init(json:)`. |
| `toBson() -> [UInt8]` | The BSON document. |
| `init(bson: [UInt8]) throws` | Reads one BSON document, exactly `bson`, as `init(json:)`. |

```swift
let msg = Telemetry()
msg.Set_device_id(42)
let text = msg.toJson()

do {
    let copy = try Telemetry(json: text)
} catch {
    // not a Telemetry
}
```

The initializers build a new message, so there is nothing to leave
unchanged on an error. The helpers are `lang/swift/SkywireText.swift`, which
ships like `SWStream.swift`: build it into the module with the generated
files, or take both from the `Skywire` package (see
[Swift package](using.md#swift-package)); wiregen does not write it. It is
Swift 5.9 or newer with only the standard library, its types all inside
`enum SkywireText`. Floats are written from their exact decimal value,
worked out here, rounded ties to even, and read with `Double(String)`, or
`Float(String)` for a `float` member, so the decimal is rounded straight to
32 bits. A member is matched by its name's exact bytes, not by Swift's
`String` equality, which would take a name that is only canonically
equivalent (KELVIN SIGN for `K`). The generated tests check the same
expected bytes as every other language, and `skywire_text_diff:swift`
checks that Swift accepts exactly the input C++ does and writes back the
same bytes.

`writeJson`, `readJson`, `writeXml`, `readXml`, `writeBson` and `readBson`
are what nested messages use. A property cannot share a method's name, so
with `--generate_json` a member cannot be named `toJson`, `writeJson` or
`readJson`; likewise for XML with `--generate_xml` and BSON with
`--generate_bson`. With any of them neither the message nor a member can be
named `SkywireText`.

## JavaScript

| Method | |
|---|---|
| `toJsonText(): string` | The JSON text. (`toJSON()`, for `JSON.stringify`, is unchanged and not this form.) |
| `static fromJsonText(text)` | Reads JSON from a string or UTF-8 bytes (a typed array, `DataView` or `ArrayBuffer`); throws `SyntaxError` if it is not the JSON form of one. A string with a lone surrogate, like bytes that are not UTF-8, is refused. |
| `toXml(): string` | The XML text. |
| `static fromXml(text)` | Reads XML, as `fromJsonText`. |
| `toBson(): Uint8Array` | The BSON document. |
| `static fromBson(bytes)` | Reads one BSON document, exactly `bytes`, as `fromJsonText`. |

```js
import { Telemetry } from "./Telemetry.js";

const msg = new Telemetry();
msg.device_id = 42;
const text = msg.toJsonText();

try {
  const copy = Telemetry.fromJsonText(text);
} catch (e) {
  // a SyntaxError: not a Telemetry
}
```

The readers build a new message, so there is nothing to leave unchanged on
an error; each value goes through its member's setter, which converts it as
C does (a `char` of 200 becomes -56). The helpers are the module
`skywire_text.js`, with its declarations `skywire_text.d.ts`, which wiregen
writes beside the modules when any of the options is given; the modules
import it as `skywireText`. It is ES2020 with no Node APIs, like the
modules, and the same for every message. Floats are written from their
exact decimal value (with `BigInt`), rounded ties to even here, and read
with `Number`; a `float` member is not read as `Math.fround(Number(text))`,
which rounds twice and is wrong where the double lands halfway between two
floats, but corrected there from the exact value. The generated tests check
the same expected bytes as every other language,
`skywire_generated_tests:JavaScriptTypes` checks the declarations, and
`skywire_text_diff:javascript` checks that JavaScript accepts exactly the
input C++ does and writes back the same bytes.

`writeJson`, `readJson`, `writeXml`, `readXml`, `writeBson` and `readBson`
are what nested messages use. With `--generate_json` a member cannot be
named `toJsonText`, `fromJsonText`, `writeJson` or `readJson`, as its
accessor would be; likewise for XML (`toXml`, ...) with `--generate_xml` and
BSON (`toBson`, ...) with `--generate_bson`. With any of them a message
cannot be named `skywireText`, the binding the helpers are imported as.
