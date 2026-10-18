# Skywire - Multilanguage Message Serialization Generator

                                        )
            _                          /
         _..\'.___.--._______.----.__.'/
    ,---',>  _/                     .'
     '',____,. __.       ..--''.    (____
            \  .___)..--'       '..____. '._
            '.__)                      '.__)

mrf

Skywire (c) 2014-2018 Skyloupe, LLC.  All rights reserved.

# [1.0] Introduction

Welcome to Skywire!

In this repository you will find a simple yet surprisingly versatile message 
serialization generator for C and C++.

Based on Google Protocol Buffer technology, Skywire takes protocol buffers to 
a place it fears to tread:  namely, the ability to define a wire protocol.

Yes, that's right.  Bits.  Endianness.  Terrible things to behold.  Oh my.

Unlike generators like Protocol Buffers, the Skywire generator (known as 
"wiregen") is asymmetric:  wiregen generated code only has to run on one 
side of the connection:  if you have an ICD for the component or device you 
are trying to communicate with, wiregen gives you the ability to define a 
"stencil" for a C++ class (or C struct) that can be used to generate code 
to interface with it.

Currently, wiregen will supports generating messages in
* C
* C++ (classical and modern)
* C# (future)
* Go
* Java
* Javascript (future)
* Python
* Rust (future)
* Swift

The message specification itself is defined using a JSON object which is in turn 
serialized into a Protocol Buffer message.  This message can be used in the
traditional Protocol Buffer manner, or a new C++ class (or C struct!) can be
generated that will give the developer the ability to serialize the message 
data "natively" for the targeted ICD or device.

The developer is given wide latitude in defining the manner and style of the 
generated code, so that the generated code can adhere to in-house naming and 
style conventions to the greatest degree possible.

In addition to this, wiregen can generate unit tests for the generated code.
While these are not necessary for generating messages using default settings,
it comes in very handy when the wiregen generator has been customized.  All 
unit tests are generated using the Google Test unit test framework.

## [1.2] Source Code Repository

At the time of this writing, the Skywire source code can be found at:

https://github.com/skyloupe/skywire

## [1.2] Dependencies

Google Protocol Buffers

# [2.0] Build Requirements

cmake version 3.28 or higher

## [2.1] Build Requirements (Windows)

One of the following development environment(s):

Visual Studio 15 Professional (or newer)

## [2.2] Build Requirements (Linux)

gcc 4.9 (or higher)

## [2.3] Build Requirements (OSX, MacOS)

XCode 8.1 (or newer)

# [3.0] Build Instructions

Skywire is heavily dependent on CMake.  No other build system is currently supported.
That said, we'd be happy to integrate others should there be enough requests for 
it.

$ cmake -S skywire -B build_dir -D CMAKE_PREFIX_PATH="/usr/local/protobuf;/usr/local/googletest" -D CMAKE_INSTALL_PREFIX=$HOME/skywire

$ cmake --build build_dir

## [3.1] Windows

	cmake -G "Visual Studio 14 2015 Win64" -S skywire -B build_dir -D CMAKE_PREFIX_PATH="C:/protobuf;C:/googletest" -D CMAKE_INSTALL_PREFIX=%HOME%/skywire 

	cmake --build build_dir --target install --config [Debug|Release|DebWithRelInfo|MinRelSize]

### [3.1.1] A note about building on Windows

What is it with Windows?  Why must already complex tasks be further muddied
with needless complexity?  Sigh...  it is a mystery.

That said, it is a mystery that we must embrace if we are to target the OS that
runs 90% of the computers on the planet.  CMake handles most of this for us, but
things become further muddled when another large company (that rhymes with "Google")
decides to "simplify" things by forcing certain compiler switches in their 
repositories... but by doing so in an inconsistent manner as is the case with 
Protocol Buffers and GTest.

Should you decide to build against either of these locally the following definition
should be invoked in CMake at build time:

-DCMAKE_PREFIX_PATH=c:\Users\username\apps\protobuf-debug;c:\Users\username\apps\gtest-debug

The paths of course must be craft to match your development harness.  Note that 
this can also be done for Linux and Mac builds if need be. 

On Linux:
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=[Debug | Release] ~/source/skywire

    cmake --build . --target install

On Mac (using Xcode):
	cmake -G "Xcode" -DCMAKE_BUILD_TYPE=[Debug | Release] ~/source/skywire

You will have to modify the paths appropriately for your system.

The following cmake generators have been tested:

    cmake -G "Visual Studio 14 2015 Win64"
    cmake -G "Visual Studio 14 2015"
    cmake -G "Xcode"
    cmake -G "Unix Makefiles"

# [4.0] Running unit tests

The unit tests are written using Google Test and are managed with CTest.  After a successful build,
type

	ctest

to run them.

The `skywire_generated_tests:*RoundTrip` tests generate each language's code
with its unit tests, then compile and run them. They need that language's
toolchain (swiftc; go 1.21 or newer; python3; a JDK; rustc 1.58 or newer) and
report Skipped without it. `RustRoundTrip` also finds a rustup install in
`~/.cargo/bin` when it isn't on `PATH`, builds with plain `rustc` (no Cargo),
and fails on any compiler warning, on output that isn't rustfmt-clean (when
rustfmt is installed), and on clippy warnings (when clippy is installed).

`skywire_ring:Serial` passes one serialized message through the generated code
of every language (C, C++, Swift, Go, Java, Python and Rust), in a random order each
run. Each step checks the message for corruption or loss. Its input file is
written by `scripts/make_ring_bin.py`. See `src/test/generated_tests/ring/README.md`
and `scripts/README.md`.

# [5.0] Deploying and running

    wiregen.exe --language c++ --input_file e:\some_directory\MyClass.json --outdir E:\some_other_directory --write_to_file

The message specification does not say which language to generate; the command
line does. `--language` takes one of `c`, `c++` (or `cpp`), `go`, `java`,
`proto3`, `python3` (or `python`) and `swift`, and uses that language's lang
file from the directory wiregen is installed in. `--lang_file <path>` uses a
different lang file instead (for example a customized copy); on its own it also
sets the language, since every lang file names its language. If both are given
they must agree. (`--langfile` is the old spelling of `--lang_file`.)

`--namespace <name>` replaces the specification's `namespace` (the C++
namespace, or the Go, Java or Python package), so one specification can be
generated into different namespaces. The specification need not have a
namespace at all when the command line gives one.

Without `--write_to_file` the generated code is printed to standard output.

# [6.0] Command-line options

| Option | Effect |
|---|---|
| `--input_file <file>` | The message specification to read. |
| `--input_str <json>` | The message specification, given on the command line instead. |
| `--language <name>` | The language to generate, using its lang file installed next to wiregen: `c`, `c++` (or `cpp`), `go`, `java`, `proto3`, `python3` (or `python`), `swift`. |
| `--lang_file <file>` | A lang file to use instead of the installed one (`--langfile` is the old spelling). |
| `--langspec <json>` | Lang file contents, given on the command line instead. |
| `--namespace <name>` | The namespace (package) to generate into, replacing the specification's. |
| `--generate_tests` | Also generate unit tests (see 7.1). |
| `--generate_relay` | Also generate a relay program (see 7.1). |
| `--write_to_file` | Write the generated files. Without it they are printed to standard output. |
| `--outdir <dir>` | The directory to write the files to (default: the current directory). |
| `--silent` | Do not print the success or failure summary. |
| `--version` | Print wiregen's version and exit. |
| `--help` | Print the options and exit. |

If an option is given twice, the last one wins. An unknown option, an option missing its
value, or a value given to an option that takes none is an error, and wiregen
generates nothing. With no arguments wiregen prints its options. On failure the
exit status is non-zero.

The version is the `VERSION` in the top-level `CMakeLists.txt`; the library
reports it as `skyloupe::version()`.

# [7.0] Syntax Overview

As stated above, the input to wiregen is a JSON file,  For example:

Invoking wiregen like so: (the example is Windows-centric)

    wiregen.exe --language c++ --generate_tests --input_file e:\example.json --outdir e:\ --write_to_file

with the file Example.json....

```
    {
       "stringValues":{  
          "classname":"Example",
          "outputtestfilename":"ExampleUnitTest.cpp",
          "namespace":"SomeNamespace"
       },
       "members":{  
          "100":{  
             "type":"float",
             "name":"myfloat100",
             "endian":1
          },    
          "200":{  
             "type":"uint32_t",
             "name":"myuint32_200",
             "endian":0
          },      
       }
    }
```

...will generate the following code:

```
    // This is the generated file (e:\Example .h)

    #pragma once

    #include "streamable.h"
    #include <stdint.h>

    namespace SomeNamespace {
        class Example : public skyloupe::Streamable {
        public:
            Example() {}
            virtual ~Example() {}
            bool serialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) const {
                str.serialize(myfloat100, skyloupe::Endianness::Little);
                str.serialize(myuint32_200, skyloupe::Endianness::Big);
                return true;
            }
            bool deserialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) {
                str.deserialize(myfloat100, skyloupe::Endianness::Little);
                str.deserialize(myuint32_200, skyloupe::Endianness::Big);
                return true;
            }
            size_t size() const {
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
            float myfloat100;
            uint32_t myuint32_200;
        };
    }
```

...along with the generated unit test:

```
    // This is the generated file (e:\ExampleUnitTest.cpp)

    #include <gmock/gmock.h>
    #include <gtest/gtest.h>
    #include "Example.h"

    TEST(ExampleTest, SerDes)
    {
        SomeNamespace::Example in, out;
        skyloupe::Stream stream;

        in.Set_myfloat100(11.1f);
        in.Set_myuint32_200(14);

        bool success = in.serialize(stream);
        ASSERT_TRUE(success);

        success = out.deserialize(stream);
        ASSERT_TRUE(success);
    }

    TEST(ExampleTest, Size)
    {
        SomeNamespace::Example msg;

        ASSERT_EQ(msg.size(), sizeof(float)+sizeof(uint32_t)+sizeof(int64_t));
    }

    TEST(ExampleTest, EqualityOperator)
    {
        SomeNamespace::Example in, out;
        skyloupe::Stream stream;

        in.Set_myfloat100(11.1f);
        in.Set_myuint32_200(14);

        bool success = in.serialize(stream);
        ASSERT_TRUE(success);

        success = out.deserialize(stream);
        ASSERT_TRUE(success);

        ASSERT_EQ(in, out);
    }
```

Note the the runtime is located in <topsrc>/lang/cpp.  (This is where the skyloupe namespace 
stuff is defined.)

Each member's key (`"100"`, `"200"` above) is its tag. Members are serialized in
numeric tag order, and the tags are also protobuf field numbers (below), so a
tag must be a valid field number: from 1 to 536870911, and not 19000–19999,
which protobuf reserves. wiregen rejects any other tag as an invalid
specification.

#### Bitfield members

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

In protobuf (PZ) mode a bitfield is an ordinary field under its own tag, of its
type's proto3 type; decoding cuts an out-of-range value to the field's bits,
as the setter does. The `.proto` comment records the layout, e.g.
`uint32 version = 1; // uint8_t:4, bits 0-3`.

#### Nested messages

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
C++, `from Inner import Inner` in Python, `import "Inner.proto";` in proto3;
Go, Java and Swift need nothing. The specification does not need to list it in
`includes`.

On the wire a nested message is its own serialization, inline, in tag order
like any member. Its members keep their own byte orders, so the nested member's
`endian` has no effect. Its size is the nested message's size, and if it fails
to deserialize, so does the message containing it.

#### Protobuf (PZ) mode

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
message (below) is an embedded message field of its own message type, imported
from its own generated `.proto`; like any zero field, it is left out when all
its members are zero, and a repeated one merges into the earlier one. PZ mode
is C++ only.

To generate the message in C, run wiregen with `--language c` and this file:

````
{
   "stringValues":{  
      "classname":"Example",
      "namespace":"SomeNamespace",
      "outputtestfilename":"ExampleUnitTest.cpp"
   },
   "members":{  
      "100":{  
         "type":"float",
         "name":"myfloat100",
         "endian":1
      },    
      "200":{  
         "type":"uint32_t",
         "name":"myuint32_200",
         "endian":0
      },
   }
}

````

The output file names come from the lang file (see 7.1); this spec names only
the test file, to replace the lang file's `ExampleTest.cpp`.

Keep in mind the JSON values are case sensitive.  You should get the files:
    Example.c
    Example.h
    and ExampleUnitTest.cpp (note that the Google Test unit tests are always C++).

Note that the C and C++ generates equivalent code.  See the generated_tests in the
source for verification of this.

# [7.1] Lang(uage) Files

Customization of generated code can be performed in the lang.xxx.json files.

Each lang file names the language it is for in its `"language"` value, which
selects the generator. wiregen ignores (with a warning) a `"language"` value in a
message specification.

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
| rust     |                 | `{classname_snake}.rs` | `{classname_snake}_test.rs` | `{classname_snake}_relay.rs` |
| swift    |                 | `{classname}.swift` | `{classname}_test.swift`| `{classname}Relay.swift`|

A message specification may set any of these keys to use a different name for
that one message; its value may also use `{classname}` or `{classname_snake}`. A name must be a plain
file name (no directories). Java requires a public class's file to be named
after the class, so the Java generator names the test and relay classes after
their files.

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

The lang file also lists the includes (imports) its generated code always needs,
such as the language's runtime. They come first in the generated file, followed
by any `includes` in the specification that are not already listed. A
specification needs `includes` only for something of its own; a nested
message's header or module is imported without it (see Nested messages).

| Language | Lang file `includes` |
|----------|----------------------|
| c        | `"cstream.h"` |
| c++      | `"streamable.h"`, `<stdint.h>` |
| swift    | `Foundation` |

The Go, Java and Python generators add the imports their code uses themselves.
The Rust generator needs no imports beyond `use super::<module>::<Type>;` for
each nested message, which it adds itself, and ignores a specification's
`includes`.

More detail on this to come.

# [Appendix] Credits

This software is dedicated to all of the former racing Greyhounds who didn't
find a home after their racing "career" ended.  If you are considering getting 
a dog, please consider rescuing a Grey.  They are awesome pets.

In the U.S. visit: http://www.ngap.org/index.html

Skywire is offered under the Apache 2.0 license.
https://www.apache.org/licenses/LICENSE-2.0
(c)2014-2020 Skyloupe, LLC.  All rights reserved.

Skywire uses and bundles the following third party software packages:

Google Protocol Buffers - (Apache 2.0)
https://developers.google.com/protocol-buffers/

Google Test - (Apache 2.0)
https://github.com/google/googletest

Qwikpirate - a command line parsing tool (LGPL)
https://github.com/skyloupe/qwikpirate
