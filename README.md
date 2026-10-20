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
serialization generator for C, C++, Go, Java, Python, Rust and Swift.

Based on Google Protocol Buffer technology, Skywire takes protocol buffers to 
a place it fears to tread:  namely, the ability to define a wire protocol.

Yes, that's right.  Bits.  Endianness.  Terrible things to behold.  Oh my.

Unlike generators like Protocol Buffers, the Skywire generator (known as 
"wiregen") is asymmetric:  wiregen generated code only has to run on one 
side of the connection:  if you have an ICD for the component or device you 
are trying to communicate with, wiregen gives you the ability to define a 
"stencil" for a message (a C++ class, a C struct, a Go struct, ...) that can
be used to generate code to interface with it.

wiregen generates messages in

* C
* C++ (which can also speak protobuf; see "Protobuf (PZ) mode")
* Go
* Java
* Python 3
* Rust
* Swift

and can also write the matching proto3 `.proto` file.

A message specification is a JSON file listing the message's members: each
one's tag, type, name and byte order, plus fixed-length strings, bitfields and
nested messages. It says nothing about the language; the command line picks
that, and a "lang file" per language holds the language's conventions. Every
language puts the same bytes on the wire, and the tests check that they do
(section 4.0).

The developer is given wide latitude in defining the manner and style of the 
generated code, so that the generated code can adhere to in-house naming and 
style conventions to the greatest degree possible.

In addition to this, wiregen can generate unit tests for the generated code
(Google Test for C and C++, each language's own test style for the others),
and a "relay" program that checks a message read from a file member by member.
While these are not necessary for generating messages using default settings,
they come in very handy when the wiregen generator has been customized.

## [1.1] Source Code Repository

At the time of this writing, the Skywire source code can be found at:

https://github.com/skyloupe/skywire

Qwikpirate (command-line parsing) is a git submodule, so clone with

    git clone --recursive https://github.com/skyloupe/skywire.git

or run `git submodule update --init` in an existing clone.

## [1.2] Dependencies

| Package | Used for |
|---|---|
| Google Protocol Buffers | wiregen reads specifications through it; the proto3 tests run its `protoc` |
| protozero | The C++ runtime: every generated C++ message has protobuf (PZ) functions, so building skywire and building any program with generated C++ both need its headers |
| Google Test | Building skywire (its tests), and building the C and C++ tests that `--generate_tests` writes |
| Qwikpirate | wiregen's command line (bundled as a submodule) |

# [2.0] Build Requirements

* CMake 3.28 or newer
* A C++17 compiler (and a C99 compiler)

Skywire is built and tested on:

| Platform | Compiler | CMake generators |
|---|---|---|
| macOS (Apple silicon) | Apple clang | Unix Makefiles |
| Ubuntu 24.04 (x86_64) | GCC | Unix Makefiles |
| Windows 11 (x64) | Visual Studio 2026 (MSVC 19.50) | Ninja, Visual Studio 18 2026 |

The tests that compile generated code in other languages need that language's
toolchain, and are skipped without it (see 4.0).

# [3.0] Build Instructions

Skywire is heavily dependent on CMake.  No other build system is currently supported.
That said, we'd be happy to integrate others should there be enough requests for 
it.

Point CMake at your protobuf and Google Test installs with `CMAKE_PREFIX_PATH`,
and at protozero with `PROTOZERO_DIR` (the directory holding its `include`):

    cmake -S skywire -B build_dir -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_PREFIX_PATH="$HOME/apps/protobuf;$HOME/apps/googletest" \
          -DPROTOZERO_DIR=$HOME/apps/protozero \
          -DCMAKE_INSTALL_PREFIX=$HOME/skywire
    cmake --build build_dir
    cmake --install build_dir

`cmake --install build_dir --prefix <dir>` installs somewhere else without
reconfiguring. Under the prefix:

| Path | Contents |
|---|---|
| `bin/` | `wiregen` (on Windows, with the protobuf and abseil DLLs it needs) |
| `include/skywire/` | The C and C++ runtimes, both header-only: `cstream.h`, `sw_endian.h`, `stream.h`, `streamable.h` |
| `share/skywire/runtime/` | The Swift runtime, `swift/SWStream.swift` |
| `share/skywire/lang/` | The lang files, `lang.*.json` |
| `lib/cmake/skywire/` | The CMake package (see 3.2). Some Linux systems use `lib64` or a multiarch directory in place of `lib`. |
| `lib/pkgconfig/` | `skywire.pc`, for builds without CMake (see 3.2) |
| `share/doc/skywire/` | `README.md` and `LICENSE` |

Nothing from the tests is installed. The generated Go, Java, Python and Rust
code needs no runtime. The build also leaves a ready-to-run `wiregen`, with its
lang files beside it, in `build_dir/bin`.

Build options:

| Option | Default | Effect |
|---|---|---|
| `SKYWIRE_EXAMPLES` | ON | Build and test the programs in `examples/` (see 4.2). |
| `SKYWIRE_BENCH` | OFF | Build the benchmarks in `bench/` (see 4.1). |
| `SKYWIRE_SANITIZE` | OFF | Build all C and C++ with AddressSanitizer and UndefinedBehaviorSanitizer (MSVC: AddressSanitizer only). Use a separate build directory. |

## [3.1] Windows

What is it with Windows?  Why must already complex tasks be further muddied
with needless complexity?  Sigh...  it is a mystery.

That said, it is a mystery that we must embrace if we are to target the OS that
runs 90% of the computers on the planet.  The easiest route is vcpkg: install
`protobuf`, `gtest` and `protozero` for the `x64-windows` triplet, then, from a
Visual Studio x64 developer prompt:

    cmake -S skywire -B build_dir -G Ninja -DCMAKE_BUILD_TYPE=Release ^
          -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
          -DVCPKG_TARGET_TRIPLET=x64-windows
    cmake --build build_dir

or `-G "Visual Studio 18 2026" -A x64` in place of `-G Ninja`, building with
`cmake --build build_dir --config Release`. vcpkg puts protozero on the include
path, so `PROTOZERO_DIR` is not needed.

To install:

    cmake --install build_dir --prefix C:/skywire

adding `--config Release` for the Visual Studio generator. `bin` gets
`libprotobuf.dll` and `abseil_dll.dll` beside `wiregen.exe`, so the installed
wiregen runs without vcpkg on `PATH`.

## [3.2] Using skywire from another project

An installed skywire is a CMake package, used much like protobuf's:

    find_package(skywire CONFIG REQUIRED COMPONENTS c)

    add_executable(app main.c)
    skywire_generate(TARGET app LANGUAGE c SPECS specs/Position.json specs/Telemetry.json)

Point `CMAKE_PREFIX_PATH` at the install prefix, and for C++ at protozero too
(`PROTOZERO_DIR`, or vcpkg):

    cmake -S app -B app_build -DCMAKE_PREFIX_PATH=$HOME/skywire \
          -DPROTOZERO_DIR=$HOME/apps/protozero

The components are `c` and `cpp`; without `COMPONENTS` you get whichever are
available. Generated C++ always needs protozero, a header-only library that is
not part of the install. The `cpp` component looks for it on the include path,
under `PROTOZERO_DIR`, or in `SKYWIRE_PROTOZERO_INCLUDE_DIR` (the directory
holding `protozero/`). Without it, `skywire::cpp_runtime` is not defined, a
`find_package` that requires `cpp` fails and says so, and
`skywire_generate(TARGET ... LANGUAGE cpp)` stops with the same advice.

The package provides:

| Name | What it is |
|---|---|
| `skywire::wiregen` | The generator, for your own `add_custom_command` |
| `skywire::c_runtime` | The header-only runtime for generated C (`cstream.h`) |
| `skywire::cpp_runtime` | The header-only runtime for generated C++, with protozero's include directory and C++17 |
| `skywire_generate()` | Generates C or C++ at build time |
| `SKYWIRE_INCLUDE_DIR`, `SKYWIRE_RUNTIME_DIR`, `SKYWIRE_LANG_DIR` | The installed headers, the Swift runtime and the lang files |

    skywire_generate(LANGUAGE <c|cpp>
                     SPECS <spec.json>...
                     [TARGET <target>]
                     [OUT_DIR <dir>]
                     [OUT_VAR <variable>]
                     [NAMESPACE <namespace>])

`skywire_generate()` runs wiregen on each specification whenever it (or
another in `SPECS`) changes. Its output goes to `OUT_DIR`, by default
`skywire_c` or `skywire_cpp` in the current binary directory. With `TARGET` the
generated sources are added to that target, which also gets `OUT_DIR` on its
include path and links the language's runtime. `OUT_VAR` receives the generated
file names. Give a nested message's specification in the same call as the
message that holds it, in the same directory. For the other languages, run
`skywire::wiregen` yourself; Swift code also needs
`${SKYWIRE_RUNTIME_DIR}/swift/SWStream.swift`.

Without CMake, use `skywire.pc`. It gives the include path, and its `wiregen`,
`langdir` and `runtimedir` variables give the generator, the lang files and the
Swift runtime. Its paths are relative to the `.pc` file, so they stay right
wherever the install is:

    export PKG_CONFIG_PATH=$HOME/skywire/lib/pkgconfig
    WIREGEN=$(pkg-config --variable=wiregen skywire)
    mkdir -p out
    $WIREGEN --language c --input_file Position.json --outdir out --write_to_file
    cc -std=c99 $(pkg-config --cflags skywire) -Iout -o app main.c out/Position.c

Neither runtime has anything to compile or link. For C++, add protozero's
include directory and `-std=c++17`; protozero has no `.pc` of its own.

Code generated with `--generate_tests` needs Google Test, for C and C++ only;
the tests for the other languages use their standard library. The C and C++
test files include `<gtest/gtest.h>` and `<gmock/gmock.h>` and have no
`main()`, so link them with `GTest::gtest_main` and `GTest::gmock` (from
`find_package(GTest)`). Swift, Go, Python, Java and Rust tests need only their
own toolchain.

`src/test/install/consumer` is a small project of this kind; ctest builds it
against a scratch install (`skywire_install:Consumer`), once through the CMake
package and once, when pkg-config is there, through `skywire.pc`.

# [4.0] Running unit tests

The tests are managed with CTest. After a successful build, run them from the
build directory:

    cd build_dir
    ctest -j8 --output-on-failure

With a multi-config generator (Visual Studio, Xcode) give the configuration
you built: `ctest -C Release`. Without it every test fails as "Not Run" (Missing
"-C <config>"). `ctest -R <regex>` runs a subset, for example
`ctest -R skywire_example`, and `ctest -N` lists the tests without running them.

Every test name starts with its group:

| Group | What it tests |
|---|---|
| `skywire_unit_test:*` | wiregen's own code: specifications, lang files and each generator (Google Test) |
| `wiregen_cli:*` | wiregen's command line: options, languages, lang files, errors |
| `qwikpirate_test:*` | The qwikpirate command-line parser (Google Test) |
| `skywire_generated_tests:*` | Generated C and C++, compiled into the build with their generated Google Test tests, plus bitfield, nested-message and PZ (protobuf) decoding checks |
| `skywire_generated_tests:*RoundTrip` | The generated Swift, Go, Python, Java and Rust code, each with its generated unit tests |
| `skywire_generated_tests:Proto3Wire` | The generated `.proto`, checked with protoc |
| `skywire_ring:Serial` | One message through every language in turn |
| `skywire_example:*` | The example programs (section 4.2); only with `SKYWIRE_EXAMPLES` |
| `skywire_install:Consumer` | The install and its CMake package (section 3.2) |

The `skywire_generated_tests:*RoundTrip` tests generate each language's code
with its unit tests, then compile and run them. They need that language's
toolchain (swiftc; go 1.21 or newer; python3; a JDK; rustc 1.58 or newer) and
report Skipped without it, as do the matching `skywire_example` tests.
`RustRoundTrip` also finds a rustup install in
`~/.cargo/bin` when it isn't on `PATH`, builds with plain `rustc` (no Cargo),
and fails on any compiler warning, on output that isn't rustfmt-clean (when
rustfmt is installed), and on clippy warnings (when clippy is installed).

`skywire_ring:Serial` passes one serialized message through the generated code
of every language (C, C++, Swift, Go, Java, Python and Rust), in a random order each
run, ending back in the language it started in. Each step checks the message for
corruption or loss. A language without its toolchain is left out of the ring
(set `SKYWIRE_RING_REQUIRE_ALL=1` to fail instead), and the test reports
Skipped with fewer than two languages. Its input file is
written by `scripts/make_ring_bin.py`. See `src/test/generated_tests/ring/README.md`
and `scripts/README.md`.

`skywire_example:<Language>` builds and runs the example programs (section 4.2).

`skywire_install:Consumer` installs the build into a scratch prefix (under
`build_dir/src/test/install/run`, never `CMAKE_INSTALL_PREFIX`), checks what
was installed, and builds and runs the C and C++ examples from a separate
project that uses the install (section 3.2), with the CMake package and with
`skywire.pc`. It also checks that a project requiring the `cpp` component
without protozero fails with a clear message.

To submit results to a CDash dashboard (`ctest -D Experimental`), put your
own `CTestConfig.cmake` in the source directory. It is not part of the
repository.

# [4.1] Benchmarks

`bench/` times the generated code of every language on the ring message (135
bytes, a different byte order on nearly every field, a nested message,
bitfields and fixed-length strings). The benchmarks are not part of ctest.
Build them in a Release directory without sanitizers:

	cmake -S skywire -B build-bench -DCMAKE_BUILD_TYPE=Release -DSKYWIRE_BENCH=ON <your usual options>
	cmake --build build-bench --target bench

or, after a build, `bench/run.sh --build build-bench [c cpp swift go python java rust]`.
`bench/README.md` says exactly what each number times and how to run them on
Windows.

Nanoseconds per message on an Apple M1 Pro (October 2026; Apple clang, Swift
6.4 with `-wmo`, Go 1.21, Python 3.14, JDK 19, Rust 1.99), serializing into a
reused buffer:

| Language | Serialize | Deserialize | Notes |
|---|---:|---:|---|
| C++ | 7.8 | 11.9 | |
| C | 7.9 | 10.3 | |
| Go | 8.8 | 8.6 | `Serialize()` to a new slice: 33 |
| Rust | 10.0 | 8.1 | `serialize()` to a new `Vec`: 26 |
| Java | 30 | 31 | `serialize()` to a new `byte[]`: 35 |
| Swift | 34 | 43 | |
| Python | 1096 | 852 | 368 / 446 for a message in one byte order |

The deserialize times for C, C++ and Swift include copying the input into the
stream. Expect Swift to be slower without `-wmo` (whole-module optimization,
the default for SwiftPM and Xcode release builds).

# [4.2] Examples

`examples/` has a small program for each language: C, C++, Swift, Go, Python,
Java and Rust. Each one fills a message that holds a nested message, writes it
to a file, reads it back and checks the copy is equal. A further C++ example,
`examples/protobuf`, sends the message through a PZ-mode stream to protoc's C++
code for the generated `.proto` and back. Both specs are in
`examples/specs/`, and each language's directory has its own README with the
wiregen and compiler commands. The examples are built and tested by ctest
(`skywire_example:<Language>`); configure with `-DSKYWIRE_EXAMPLES=OFF` to
leave them out. See `examples/README.md`.

# [5.0] Deploying and running

    wiregen --language c++ --input_file specs/MyClass.json --outdir out --write_to_file

The message specification does not say which language to generate; the command
line does. `--language` takes one of `c`, `c++` (or `cpp`), `go`, `java`,
`proto3`, `python3` (or `python`), `rust` and `swift`, and uses that language's lang
file from wiregen's own directory or, in an install, from `share/skywire/lang`. `--lang_file <path>` uses a
different lang file instead (for example a customized copy); on its own it also
sets the language, since every lang file names its language. If both are given
they must agree. (`--langfile` is the old spelling of `--lang_file`.)

`--namespace <name>` replaces the specification's `namespace` (the C++
namespace, or the Go, Java or Python package), so one specification can be
generated into different namespaces. The specification need not have a
namespace at all when the command line gives one. Rust has no use for it: a
Rust module's path is wherever you declare it with `mod`.

Without `--write_to_file` the generated code is printed to standard output.

# [6.0] Command-line options

| Option | Effect |
|---|---|
| `--input_file <file>` | The message specification to read. |
| `--input_str <json>` | The message specification, given on the command line instead. |
| `--language <name>` | The language to generate, using its installed lang file (see 5.0): `c`, `c++` (or `cpp`), `go`, `java`, `proto3`, `python3` (or `python`), `rust`, `swift`. |
| `--lang_file <file>` | A lang file to use instead of the installed one (`--langfile` is the old spelling). |
| `--langspec <json>` | Lang file contents, given on the command line instead. |
| `--namespace <name>` | The namespace (package) to generate into, replacing the specification's. |
| `--generate_tests` | Also generate unit tests (see 7.1). The C and C++ tests need Google Test (see 3.2). |
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

As stated above, the input to wiregen is a JSON file. For example, `Example.json`:

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

`endian` is 0 for big-endian (the default), 1 for little-endian and 2 for the
host's own order. Keep in mind the JSON values are case sensitive. Running

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

The PZ functions are the protobuf encoding (see "Protobuf (PZ) mode"); the
`*_sw` functions write and read memory directly (see "Serializing in place").
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

The same specification with `--language c` gives `Example.h` and `Example.c`:
a plain `Example` struct with `Example_Serialize`, `Example_Deserialize`,
`Example_Size`, `Example_Equals` and getters and setters, using the C runtime
in `lang/c` (`cstream.h`, header-only); its generated tests are also Google
Test C++. The C and C++ code puts the same bytes on the wire; see
`src/test/generated_tests` for the proof. `examples/` has a complete program
in every language.

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
| Rust | No: the fields are private to the message's module. |

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
C++, `from Inner import Inner` in Python, `import "Inner.proto";` in proto3,
`use super::inner::Inner;` in Rust; Go, Java and Swift need nothing. The
specification does not need to list it in `includes`.

On the wire a nested message is its own serialization, inline, in tag order
like any member. Its members keep their own byte orders, so the nested member's
`endian` has no effect. Its size is the nested message's size, and if it fails
to deserialize, so does the message containing it.

#### Serializing in place

In C, C++, Swift, Go and Rust the generated code handles a whole message at
once: it claims `size()` bytes from the stream (or slice), then stores each
member at its offset in the requested byte order, with no per-member calls into
the runtime; reading is the same in reverse. Python packs and unpacks one
`struct` per byte order, and Java uses `ByteBuffer`. The functions that work in
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

The C, C++ and Swift in-place functions don't check bounds: give them at least
`size()` bytes. They return (C, C++) or advance `p` to (Swift) the end of what
they wrote or read. Go's `Put` and Rust's `write_to` panic on a slice shorter
than the message. A nested message is written and read in place by the same
functions.

Notes:

* C `X_Serialize` returns -1 if the stream cannot grow.
* C++ `Stream::clear()` keeps the buffer, so writes after it neither reallocate
  nor zero-fill.
* Both the C and C++ runtimes are header-only.
* Swift `SWStream.byteSize()` is the number of unread bytes, the same as
  `remaining()`. A stream used for many messages reuses the space it has read.
* Go's `XSize` constant (e.g. `RingyMcRingfaceSize`) is a package-level name.
* Java's `hashCode()` gives the same values as `Objects.hash` of the members,
  without boxing them.

#### Rust

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
message (see Nested messages) is an embedded message field of its own message type, imported
from its own generated `.proto`; like any zero field, it is left out when all
its members are zero, and a repeated one merges into the earlier one. PZ mode
is C++ only. `examples/protobuf` (section 4.2) is a complete program that talks
to protoc's generated C++ through a PZ stream.

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
