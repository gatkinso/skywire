# Specifications

A specification is a JSON file that describes one message: its class name and
its members, in the order they go on the wire. It says nothing about the
language; the command line picks that. `examples/specs/Telemetry.json`:

```json
{
   "stringValues": {
      "classname": "Telemetry",
      "namespace": "telemetry",
      "bitorder": "lsb"
   },
   "members": {
      "1": { "type": "uint32_t", "name": "device_id",    "endian": 0 },
      "2": { "type": "uint64_t", "name": "timestamp_us", "endian": 1 },
      "3": { "type": "uint8_t",  "name": "mode",         "bits": 4 },
      "4": { "type": "uint8_t",  "name": "status",       "bits": 3 },
      "5": { "type": "uint8_t",  "name": "armed",        "bits": 1 },
      "6": { "type": "string",   "name": "callsign",     "length": 8 },
      "7": { "type": "Position", "name": "position" },
      "8": { "type": "uint16_t", "name": "battery_mv",   "endian": 1 }
   }
}
```

On the wire this is a big-endian `uint32_t`, a little-endian `uint64_t`, one
byte holding three bitfields, 8 bytes of string, a `Position` message, and a
little-endian `uint16_t`: no padding, no length or type bytes, nothing that
isn't a member.

wiregen reads a specification as the protobuf JSON form of
`Specification` in `src/lib/skywireproto/specification.proto`. Names and
values are case sensitive. An unknown field, at the top level or in a member,
is an error, but `stringValues` may hold keys wiregen doesn't use.

| Field | Holds |
|---|---|
| `stringValues` | The message's settings, below. `classname` is required. |
| `members` | The members, keyed by tag. At least one. |
| `includes` | Extra includes or imports for the generated code (see [Includes](#includes)). |
| `$schema` | The [JSON Schema](#editor-support), for editors. wiregen ignores it. |

(As in any protobuf JSON, `string_values` is accepted for `stringValues`.)

## Settings

| `stringValues` key | Meaning |
|---|---|
| `classname` | The message's class (struct, type) name. Required; an identifier. |
| `namespace` | The C++ or C# namespace, or the Go, Java or proto3 package: identifiers joined by `.` or `::`. Optional; `wiregen --namespace` replaces it. C, Python, Rust, Swift and JavaScript don't use it. |
| `bitorder` | `lsb` (the default) or `msb`: where a storage unit's first [bitfield](#bitfield-members) goes. |
| `outputheaderfilename`, `outputimplfilename`, `outputtestfilename`, `outputrelayfilename` | Output file names for this message, in place of the [lang file's](lang-files.md#output-file-names). |
| `language` | Ignored, with a warning. The language comes from the command line. |

An identifier is a letter or `_` followed by letters, digits and `_`.

## Members

Each member's key is its tag. Members go on the wire in numeric tag order,
whatever order the file lists them in, so tags can leave gaps for members
added later. The tags are also protobuf field numbers (see
[Protobuf (PZ) mode](protobuf.md)), so a tag must be a valid field number:
from 1 to 536870911, and not 19000–19999, which protobuf reserves.

| Field | Meaning |
|---|---|
| `type` | Required. A type from the table below, `string`, or another message's `classname`. |
| `name` | Required. An identifier, different from every other member's. Each language also rejects names that would collide with what it generates; see [Languages](languages.md). |
| `endian` | Byte order: `0` big-endian (the default), `1` little-endian, `2` the host's own order. No effect on 8-bit types or [nested messages](#nested-messages). |
| `length` | For a `string`, its size in bytes, at least 1. |
| `bits` | Makes an integer member a [bitfield](#bitfield-members) this many bits wide. |

| Type | Bytes on the wire |
|---|---|
| `char`, `int8_t`, `unsigned char`, `uint8_t` | 1 |
| `int16_t`, `uint16_t` | 2 |
| `int32_t`, `uint32_t`, `float` | 4 |
| `int64_t`, `uint64_t`, `double` | 8 |
| `string` | `length` |
| a message | that message's size |

`float` and `double` are IEEE 754, sent as their bits. A `string` is a
fixed-size byte array, not a length-prefixed string: a shorter value is
padded with zero bytes, and a longer one is cut to `length`.

`endian: 2` writes in the byte order of whatever machine runs the code, so the
bytes differ between machines. Use it only for data that stays on one machine.

## Includes

`includes` lists anything the generated code must include or import beyond
what the lang file already does: `<path>`, `"path"` or a plain path, as the
language expects. Each lang file's own includes (its runtime) come first.
Nested messages are imported without being listed. Rust ignores `includes`;
JavaScript turns them into side-effect imports and C# into `using` lines.

## Bitfield members

An integer member with `bits` takes only that many bits on the wire, packed
with its neighbours into a storage unit:

```
"stringValues": { "classname": "Header", "bitorder": "lsb" },
"members": {
   "1": { "type": "uint8_t",  "name": "version", "bits": 4 },
   "2": { "type": "uint8_t",  "name": "flags",   "bits": 4 },
   "3": { "type": "uint16_t", "name": "id",      "bits": 12, "endian": 0 },
   "4": { "type": "int16_t",  "name": "delta",   "bits": 4,  "endian": 0 }
}
```

- `bits` is allowed on `uint8_t` … `int64_t` only, from 1 to the type's width.
- Walking the members in tag order, a bitfield joins the open unit if it has
  the unit's type and `endian` and fits in the bits left. Otherwise it starts
  a new unit; fields never straddle units. Any other member closes the unit.
  Above, `version` and `flags` fill one byte. `id` starts a big-endian
  `uint16_t`, and `delta`, a different type, starts another.
- A unit goes on the wire as one integer of its type, in its `endian` (as for
  any member, endian has no effect on an 8-bit unit). Unused bits are written
  as 0 and ignored on read.
- `bitorder` (a `stringValues` entry, default `lsb`) sets where the first field
  goes: `lsb` puts it in the unit's low bits (as GCC and Clang do on
  little-endian hosts), `msb` in its high bits (as protocol diagrams draw it).
- In every language the member is held in its full type (in C a plain struct
  field, not a C bitfield). Setters keep only the field's bits, and signed
  fields are sign-extended: `Set_delta(-3)` reads back -3, `Set_flags(0x1f)`
  reads back 15.

**Caution: writing a bitfield member without its setter.** In some languages
the member can be assigned directly, skipping the setter's masking. Serialize
masks again, so an out-of-range value never spills into its neighbours. But
the message keeps the value as written: `Equals`, the getter and printing all
see it, while the wire carries only the low bits. A message that goes through
serialize and deserialize then no longer equals the original, and nothing
warns about it. Use the setters, or keep values in range:

| Language | Direct writes possible? |
|---|---|
| C | Yes: the members are plain struct fields (`msg.version = 20;`). |
| Python | Yes: the members are public attributes (`msg.version = 20`). |
| Go | Within the message's own package: the fields are unexported. |
| Swift | Within the module that compiles the message: the stored properties are `internal`. |
| C++ | No: the members are `private`. |
| Java | No: the fields are `private`. |
| C# | No: the fields are `private`, and the properties' setters mask. |
| JavaScript | Yes, through the backing properties (`msg._version = 20`); the accessors (`msg.version = 20`) mask. |
| Rust | No: the fields are private to the message's module. |

In protobuf (PZ) mode a bitfield is an ordinary field under its own tag, of its
type's proto3 type; decoding cuts an out-of-range value to the field's bits,
as the setter does. The `.proto` comment records the layout, e.g.
`uint32 version = 1; // uint8_t:4, bits 0-3`.

## Nested messages

A member's type can be another generated message: any type that is not a
native type or `string` names a message, by its class name, in the same
namespace (package, module).

```
"members": {
   "1": { "type": "uint8_t", "name": "count" },
   "2": { "type": "Inner",   "name": "inner" }
}
```

Generate `Inner` from its own specification, into the same place. The generated
code imports it where the language needs that: `#include "Inner.h"` in C and
C++, `from Inner import Inner` in Python, `import "Inner.proto";` in proto3,
`use super::inner::Inner;` in Rust, `import { Inner } from "./Inner.js";` in
JavaScript; C#, Go, Java and Swift need nothing. The
specification does not need to list it in `includes`.

On the wire a nested message is its own serialization, inline, in tag order
like any member. Its members keep their own byte orders, so the nested member's
`endian` has no effect. Its size is the nested message's size, and if it fails
to deserialize, so does the message containing it.

## Editor support

`schema/skywire-spec.schema.json` is a JSON Schema (draft 2020-12) for
specifications. An install has it in `share/skywire/schema/`, beside
`skywire-lang.schema.json` for [lang files](lang-files.md#schema). An editor
that knows the schema completes field names and types, shows what each one
means, and marks most mistakes as you type.

Name it in the specification's `"$schema"`, by a path relative to the file
or a URL:

```json
{
   "$schema": "../schema/skywire-spec.schema.json",
   "stringValues": { "classname": "Position" },
   ...
}
```

wiregen ignores a top-level `"$schema"` (from 0.10.0; earlier versions reject
it). To use the schema without editing each file, map it to your specs in the
editor instead. In VS Code's `settings.json`:

```json
"json.schemas": [
   { "fileMatch": ["specs/*.json"], "url": "./schema/skywire-spec.schema.json" }
]
```

In JetBrains IDEs, add it under Settings > Languages & Frameworks > Schemas
and DTDs > JSON Schema Mappings.

The schema can also check specifications in a build or CI, with any JSON
Schema validator, for example:

    pipx run check-jsonschema --schemafile schema/skywire-spec.schema.json specs/*.json

It checks everything wiregen checks in a single file except that member names
are different from each other and from what each language generates. wiregen
also checks what needs more than one file: that a nested message's
specification exists, has that `classname`, and doesn't contain itself.
`skywire_schema:Check` (see [Testing](testing.md#test-groups)) keeps the
schema and wiregen in agreement.
