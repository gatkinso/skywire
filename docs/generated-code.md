# Generated code

What wiregen writes for a message, using this [specification](specs.md),
`Example.json`:

```json
{
   "stringValues": {
      "classname": "Example",
      "namespace": "SomeNamespace"
   },
   "members": {
      "100": { "type": "float",    "name": "myfloat100",   "endian": 1 },
      "200": { "type": "uint32_t", "name": "myuint32_200", "endian": 0 }
   }
}
```

## C++

Running

    wiregen --language c++ --generate_tests --input_file Example.json --outdir out --write_to_file

writes `out/Example.h`:

```cpp
// This is the generated file

#pragma once

#include "streamable.h"
#include <stdint.h>

namespace SomeNamespace {
    class Example : public skyloupe::Streamable {
    public:
        Example() {}
        virtual ~Example() {}
        bool serialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) const {
            if (str.mode() == skyloupe::Stream::PZ) {
                protozero::pbf_writer pbf{str.pz_buffer()};
                return serialize_pb(pbf);
            }
            char* p = str.claim(size());
            if (p == nullptr) return false;
            serialize_sw(p);
            return true;
        }
        // Writes the SW wire form, size() bytes, at p; returns the end.
        char* serialize_sw(char* p) const {
            p = skyloupe::wire::put(p, myfloat100, skyloupe::Endianness::Little);
            p = skyloupe::wire::put(p, myuint32_200, skyloupe::Endianness::Big);
            return p;
        }
        bool deserialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) {
            if (str.mode() == skyloupe::Stream::PZ) {
                Example decoded;
                try {
                    if (false == decoded.deserialize_pb(protozero::pbf_reader{str.pz_buffer()})) return false;
                } catch (const protozero::exception&) {
                    return false;
                }
                *this = decoded;
                str.clear();
                return true;
            }
            const char* p = str.consume(size());
            if (p == nullptr) return false;
            deserialize_sw(p);
            return true;
        }
        // Reads the SW wire form, size() bytes, from p; returns the end.
        const char* deserialize_sw(const char* p) {
            p = skyloupe::wire::get(p, myfloat100, skyloupe::Endianness::Little);
            p = skyloupe::wire::get(p, myuint32_200, skyloupe::Endianness::Big);
            return p;
        }
        bool serialize_pb(protozero::pbf_writer& pbf) const {
            if (skyloupe::pb_nonzero(myfloat100)) pbf.add_float(100, myfloat100);
            if (skyloupe::pb_nonzero(myuint32_200)) pbf.add_uint32(200, myuint32_200);
            return true;
        }
        bool deserialize_pb(protozero::pbf_reader pbf) {
            while (pbf.next()) {
                switch (pbf.tag_and_type()) {
                case protozero::tag_and_type(100, protozero::pbf_wire_type::fixed32): myfloat100 = static_cast<float>(pbf.get_float()); break;
                case protozero::tag_and_type(200, protozero::pbf_wire_type::varint): myuint32_200 = static_cast<uint32_t>(pbf.get_uint32()); break;
                default: pbf.skip(); break;
                }
            }
            return true;
        }
        static size_t size() {
            return(
                sizeof(float) +
                sizeof(uint32_t)
            );
        }
        bool operator==(const Example& other) const {
            if(this == &other) return true;
            return(
                myfloat100 == other.myfloat100 &&
                myuint32_200 == other.myuint32_200
            );
        }
        float Get_myfloat100() const { return myfloat100; }
        uint32_t Get_myuint32_200() const { return myuint32_200; }
        void Set_myfloat100(const float newval) { myfloat100 = newval; }
        void Set_myuint32_200(const uint32_t newval) { myuint32_200 = newval; }
    private:
        float myfloat100{};
        uint32_t myuint32_200{};
    };
}
```

With `--generate_json`, `--generate_xml` or `--generate_bson` the class also
has `to_json` and `from_json`, `to_xml` and `from_xml`, or `to_bson` and
`from_bson` (see [JSON, XML and BSON](text-formats.md)); C gets `_ToJson`,
`_FromJson` and the rest as functions (see [C](text-formats.md#c)), Python
the same method names (see [Python](text-formats.md#python)), Go
`ToJSON`, `FromJSON`, `MarshalBSON` and the rest (see [Go](text-formats.md#go)),
C# `ToJson`, `FromJson`, `ToBson` and the rest (see [C#](text-formats.md#c_2)),
Rust the same names as C++ (see [Rust](text-formats.md#rust)), Java
`toJson`, `fromJson`, `toBson` and the rest (see [Java](text-formats.md#java)),
and Swift `toJson`, `init(json:)`, `toBson` and the rest (see [Swift](text-formats.md#swift)).

The PZ functions are the protobuf encoding (see [Protobuf (PZ) mode](protobuf.md)); the
`*_sw` functions write and read memory directly (see [Serializing in place](#serializing-in-place)).
The runtime, `skyloupe::Stream` and friends, is the header-only `lang/cpp`
(`streamable.h`, `stream.h`), which needs protozero's headers.

`--generate_tests` also writes `out/ExampleTest.cpp`, a Google Test file
(link it with `GTest::gtest_main` and `GTest::gmock`), which begins:

```cpp
// This is the generated file

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include "Example.h"

TEST(ExampleTest, SerDes)
{
    SomeNamespace::Example in, out;
    skyloupe::Stream stream;

    in.Set_myfloat100(3.5);
    in.Set_myuint32_200(3000000000);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);
}

TEST(ExampleTest, Size)
{
    SomeNamespace::Example msg;

    ASSERT_EQ(msg.size(), sizeof(float)+sizeof(uint32_t));
}

TEST(ExampleTest, EqualityOperator)
{
    SomeNamespace::Example in, out;
    skyloupe::Stream stream;

    in.Set_myfloat100(3.5);
    in.Set_myuint32_200(3000000000);

    bool success = in.serialize(stream);
    ASSERT_TRUE(success);

    success = out.deserialize(stream);
    ASSERT_TRUE(success);

    ASSERT_EQ(in, out);
}
```

and goes on to check a short buffer, the exact wire bytes, and the PZ encoding.

## C

The same specification with `--language c` gives `Example.h` and `Example.c`:
a plain `Example` struct with `Example_Serialize`, `Example_Deserialize`,
`Example_Size`, `Example_Equals` and getters and setters, using the C runtime
in `lang/c` (`cstream.h`, header-only); its generated tests are also Google
Test C++. The C and C++ code puts the same bytes on the wire; see
`src/test/generated_tests` for the proof.

## Other languages

Every language has the same members, accessors, size, equality and
serialize and deserialize functions, named in the language's own style.
[Languages](languages.md) covers Rust, C# and JavaScript in detail, and
`examples/` has a complete program in every language.

## Serializing in place

In C, C++, Swift, Go and Rust the generated code handles a whole message at
once: it claims `size()` bytes from the stream (or slice), then stores each
member at its offset in the requested byte order, with no per-member calls into
the runtime; reading is the same in reverse. Python packs and unpacks one
`struct` per byte order, Java uses `ByteBuffer`, C# writes each member
into a `byte[]` at an offset, and JavaScript through a `DataView` at an offset. The functions that work in
place are public, so a message can also be written into, or read from, memory
you manage:

| Language | Stream functions | In place | Runtime |
|---|---|---|---|
| C | `X_Serialize`, `X_Deserialize` | `char* X_SerializeSw(const X*, char*)`, `const char* X_DeserializeSw(X*, const char*)` | `cstream_claim`, `cstream_consume`; `sw_put`, `sw_get` in `cstream.h` |
| C++ | `serialize(Stream&)`, `deserialize(Stream&)` | `char* serialize_sw(char*) const`, `const char* deserialize_sw(const char*)` | `Stream::claim`, `Stream::consume` (SW mode); `skyloupe::wire::put`, `get` |
| Swift | `serialize(stream)`, `deserialize(stream)` | `serialize(into: &p)`, `deserialize(from: &p)` | `SWStream.claim`, `consume`, `clear`, `reserve`; `SWWire.put`, `get` |
| Go | `Serialize()`, `AppendTo(o)`, `Deserialize(o)` | `Put(b)` | `const XSize` |
| Rust | `serialize()`, `serialize_into(&mut Vec)`, `deserialize(&[u8])` | `write_to(&mut [u8])` | `X::SIZE` |
| Python | `serialize()`, `deserialize(buffer)` | | `X.SIZE` |
| Java | `serialize()`, `serialize(ByteBuffer)`, `deserialize(...)` | | |
| C# | `Serialize()`, `Deserialize(byte[])` | `Serialize(byte[], ref int offset)`, `Deserialize(byte[], ref int offset)` | `Size()` |
| JavaScript | `serialize()`, `deserialize(bytes)`, `fromBytes(bytes)` | `serializeInto(buf, offset)`, `deserializeFrom(buf, offset)` | `X.SIZE` |

The C, C++ and Swift in-place functions don't check bounds: give them at least
`size()` bytes. They return (C, C++) or advance `p` to (Swift) the end of what
they wrote or read. Go's `Put` and Rust's `write_to` panic on a slice shorter
than the message, C#'s throw `ArgumentException` if fewer than `Size()`
bytes remain, and JavaScript's throw `RangeError`. A nested message is written and read in place by the same
functions.

### Notes

* C `X_Serialize` returns -1 if the stream cannot grow.
* C++ `Stream::clear()` keeps the buffer, so writes after it neither reallocate
  nor zero-fill.
* Both the C and C++ runtimes are header-only.
* Swift `SWStream.byteSize()` is the number of unread bytes, the same as
  `remaining()`. A stream used for many messages reuses the space it has read.
* Go's `XSize` constant (e.g. `RingyMcRingfaceSize`) is a package-level name.
* Java's `hashCode()` gives the same values as `Objects.hash` of the members,
  without boxing them.
