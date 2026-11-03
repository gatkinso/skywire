# Protobuf (PZ) mode

A generated C++ message can also travel as protobuf, so it can talk directly to
an existing protobuf service. Pick the stream when you make it:

```
skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);   // protobuf
skyloupe::Stream stream;                                     // skywire (SW)
```

In PZ mode each member is the protobuf field numbered by its tag, and
`wiregen --language proto3` writes the matching `.proto`:

| Spec type | proto3 type | Notes |
|-----------|-------------|-------|
| `char`, `int8_t`, `int16_t`, `int32_t` | `int32` | |
| `unsigned char`, `uint8_t`, `uint16_t`, `uint32_t` | `uint32` | |
| `int64_t` / `uint64_t` | `int64` / `uint64` | |
| `float` / `double` | `float` / `double` | |
| `string` (`char[N]`) | `bytes` | sent without trailing NULs |
| bitfield (`"bits": N`) | its type's proto3 type | one field per member; decoded values are cut to N bits |

The encoding is canonical proto3, byte for byte what protoc produces: zero
values and empty strings are left out (`-0.0` is still sent). Decoding takes
fields in any order, treats missing ones as zero, lets a repeated field's last
value win, and skips unknown fields and fields with an unexpected wire type.
A wider value than the member holds is narrowed (as protobuf C++ narrows
int32 to a smaller C type); a bytes value longer than the string member fails,
a shorter one is zero-padded. On failure the message is left unchanged.
Endianness has no effect in PZ mode; the `.proto` notes each member's C type,
byte order, string length and bitfield layout in a comment.

The namespace becomes the protobuf package (`a::b` becomes `a.b`). A nested
message (see [Nested messages](specs.md#nested-messages)) is an embedded message field of its own message type, imported
from its own generated `.proto`; like any zero field, it is left out when all
its members are zero, and a repeated one merges into the earlier one. PZ mode
is C++ only. `examples/protobuf` (see [Examples](testing.md#examples)) is a complete program that talks
to protoc's generated C++ through a PZ stream.
