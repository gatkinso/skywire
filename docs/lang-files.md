# Lang files

Each language's conventions are in its lang file, `lang.<language>.json`:
the templates its code is generated from, its output file names, and the
includes every file needs. wiregen uses the installed one for `--language`;
`--lang_file` gives it another, such as a copy customized to your naming and
style conventions. The shipped lang files are in `lang/` in the source and
`share/skywire/lang/` in an install.

A lang file's templates mark where generated text goes with tokens such as
`__MEMBERNAME__`; the `*_substr` and `*_token` values name them. wiregen
replaces every occurrence of a token as literal text, not as a regular
expression, so a token may contain any characters but must not be empty.

Each lang file names the language it is for in its `"language"` value, which
selects the generator. wiregen ignores (with a warning) a `"language"` value in a
message specification.

## Output file names

The lang file also names each output file, with `{classname}` standing for the
message's `classname` and `{classname_snake}` for it in snake_case
(`RingyMcRingface` becomes `ringy_mc_ringface`):

| Language | Header (`outputheaderfilename`) | Implementation (`outputimplfilename`) | Test (`outputtestfilename`) | Relay (`outputrelayfilename`) |
|----------|-------------|---------------------|-------------------------|------------------------|
| c        | `{classname}.h` | `{classname}.c`     | `{classname}Test.cpp`   | `{classname}Relay.c`   |
| c++      | `{classname}.h` |                     | `{classname}Test.cpp`   | `{classname}Relay.cpp` |
| go       |                 | `{classname}.go`    | `{classname}_test.go`   | `{classname}Relay.go`  |
| java     |                 | `{classname}.java`  | `{classname}Test.java`  | `{classname}Relay.java`|
| proto3   |                 | `{classname}.proto` | `{classname}Test.txtpb` |                        |
| python3  |                 | `{classname}.py`    | `{classname}Test.py`    | `{classname}Relay.py`  |
| csharp   |                 | `{classname}.cs`    | `{classname}Test.cs`    |                        |
| javascript | `{classname}.d.ts` | `{classname}.js` | `{classname}.test.js` | `{classname}Relay.js` |
| rust     |                 | `{classname_snake}.rs` | `{classname_snake}_test.rs` | `{classname_snake}_relay.rs` |
| swift    |                 | `{classname}.swift` | `{classname}_test.swift`| `{classname}Relay.swift`|

A message specification may set any of these keys to use a different name for
that one message; its value may also use `{classname}` or `{classname_snake}`. A name must be a plain
file name (no directories). Java requires a public class's file to be named
after the class, so the Java generator names the test and relay classes after
their files.

## Tests and relays

`--generate_tests` writes the unit test file, and `--generate_relay` writes the
relay: a small program that reads a serialized message from a file, checks
every member against shared test values, and writes it back out. It checks the
members inside nested messages too, so wiregen reads each nested type's
specification from `<type>.json` in the same directory as the specification
it is given (`--generate_relay` fails if one is missing). The
cross-language ring test (`src/test/generated_tests/ring`) chains the relays of
every language. Naming a test or relay file in a specification does not by itself
generate that file; only the flags do. proto3 has no relay; its test file is a
text-format message holding the shared test values, headed by the bytes protoc
must encode it to (`src/test/generated_tests/proto3_roundtrip.cmake` checks
this).

## JSON, XML and BSON

With `--generate_json`, `--generate_xml` or `--generate_bson` the C and C++
generators use the `to_json_*`, `from_json_*`, `to_xml_*`, `from_xml_*`,
`to_bson_*` and `from_bson_*` templates: for each function a `_str` that
begins it, a `_end_str` that ends it, and one `_native_mem_str`,
`_string_mem_str` and `_user_mem_str` per member kind (the `from_` ones also a
`_bits_mem_str`, with `__BITS__` for the bitfield's width), and
`text_include_str` and `bson_include_str` for the runtime headers. The C
generator also takes `json_decl_str`, `xml_decl_str` and `bson_decl_str`, the
header's declarations, and `__TYPEID__`, the member's type with spaces as
underscores (`unsigned_char`), which names the runtime function that writes
or reads it (`sw_json_put_unsigned_char`). Without the
options they are not used, so a lang file without them still works. See
[JSON, XML and BSON](text-formats.md).

## Includes

The lang file also lists the includes (imports) its generated code always needs,
such as the language's runtime. They come first in the generated file, followed
by any `includes` in the specification that are not already listed. A
specification needs `includes` only for something of its own; a nested
message's header or module is imported without it (see [Nested messages](specs.md#nested-messages)).

| Language | Lang file `includes` |
|----------|----------------------|
| c        | `"cstream.h"` |
| c++      | `"streamable.h"`, `<stdint.h>` |
| swift    | `Foundation` |

The C#, Go, Java, JavaScript and Python generators add the imports (C# `using`
lines) their code uses themselves.
The Rust generator needs no imports beyond `use super::<module>::<Type>;` for
each nested message, which it adds itself, and ignores a specification's
`includes`.

## Schema

`schema/skywire-lang.schema.json` (`share/skywire/schema/` in an install)
checks a lang file's shape: its `language`, output file names and
`native_types`. Which templates a generator needs is up to the generator, so
the schema can't tell whether a customized lang file has them all; wiregen
reports a missing one as an invalid lang file. Name the schema with a
`"$schema"` line, as for a [specification](specs.md#editor-support).
