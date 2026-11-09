# JSON and XML

Besides the binary wire format, a message can be written as, and read from,
JSON or XML text: for logs, test fixtures, configuration, or a web service.
Each is asked for separately:

    wiregen --language c++ --generate_json --generate_xml --input_file Telemetry.json --outdir out --write_to_file

`--generate_json` adds `to_json` and `from_json`, `--generate_xml` adds
`to_xml` and `from_xml`. Without them nothing changes: the generated code is
exactly what it was. With `--generate_tests` the generated tests also check
the text forms.

The text forms are C++ only for now; the other languages refuse the options
until they have them. Every language will write the same text for the same
message, byte for byte, as they already put the same bytes on the wire, so
this page is their specification.

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

## Reading

`from_json` and `from_xml` read the text forms, and more besides, so text
written by hand or by another JSON or XML library is accepted:

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
- In XML the root element must be named after the class. Comments,
  processing instructions, CDATA sections, the five predefined entities and
  character references are understood; namespaces are not, and attributes
  other than `encoding` are ignored. Surrounding whitespace in a number is
  ignored, but not in a string.

Anything else fails: text that is not JSON or not well-formed XML, a number
out of range or not whole, a string too long, a value of the wrong kind
(`true`, `null`, an array, an element holding elements where a number
belongs, text where a nested message belongs), text that is not UTF-8. **On
failure the message is left unchanged.**

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

```cpp
telemetry::Telemetry msg;
msg.Set_device_id(42);
std::string json = msg.to_json();

telemetry::Telemetry copy;
if (false == copy.from_json(json)) { /* not a Telemetry */ }
```

The runtime is the header-only `lang/cpp/text.h` (installed with `stream.h`),
which uses only the standard library; it is included only when a text form is
asked for. A message's `to_json(skyloupe::text::JsonWriter&)`,
`from_json(const skyloupe::text::JsonValue&)`,
`to_xml(skyloupe::text::XmlWriter&, std::string_view element)` and
`from_xml(const skyloupe::text::XmlNode&)` are what nested messages use, and
can put a message inside a larger document of your own.

A member cannot be named `to_json` or `from_json` with `--generate_json`, or
`to_xml` or `from_xml` with `--generate_xml`.
