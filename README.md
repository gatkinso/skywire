# Skywire - Multilanguage Message Serialization Generator

[![CI](https://github.com/gatkinso/skywire/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/gatkinso/skywire/actions/workflows/ci.yml)
[![Coverage](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/gatkinso/skywire/badges/coverage.json)](https://github.com/gatkinso/skywire/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/gatkinso/skywire)](https://github.com/gatkinso/skywire/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/gatkinso/skywire/total)](https://github.com/gatkinso/skywire/releases)
[![License: Apache 2.0](https://img.shields.io/github/license/gatkinso/skywire)](LICENSE)
[![Stars](https://img.shields.io/github/stars/gatkinso/skywire)](https://github.com/gatkinso/skywire/stargazers)
[![Generates C, C++, C#, Go, Java, JavaScript, Python, Rust, Swift](https://img.shields.io/badge/generates-C%20%7C%20C%2B%2B%20%7C%20C%23%20%7C%20Go%20%7C%20Java%20%7C%20JavaScript%20%7C%20Python%20%7C%20Rust%20%7C%20Swift-informational)](docs/languages.md)

                                        )
            _                          /
         _..\'.___.--._______.----.__.'/
    ,---',>  _/                     .'
     '',____,. __.       ..--''.    (____
            \  .___)..--'       '..____. '._
            '.__)                      '.__)

mrf

Copyright 2014-2026 Geoffrey Atkinson. Licensed under the Apache License 2.0
(see LICENSE).

wiregen generates the code to read and write binary messages in a format you
already have (a device's ICD, a radio link, another team's packed structs)
in C, C++, C#, Go, Java, JavaScript, Python, Rust and Swift, and every
language puts exactly the same bytes on the wire.

Yes, that's right. Bits. Endianness. Terrible things to behold. Oh my.

You describe a message once, in JSON: its members in order, each one's type
and byte order, plus fixed-length strings, bitfields and nested messages.
wiregen turns that into plain structs or classes with getters, setters,
serialize, deserialize, equality and size, and, if you like, unit tests for
them. Most languages need no runtime library, and the generated code is yours
under any license. Generated C++ can also speak protobuf.

skywire handles fixed layouts. If your format has arrays, variable-length or
optional fields, or you get to design the format yourself, see
[Why skywire?](docs/why.md) for what to use instead.

## Quick start

Say a temperature sensor's interface control document (ICD) describes the
reading it sends, 9 bytes, big-endian:

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 2 | Sensor ID | unsigned |
| 2 | 4 | Timestamp | unsigned, milliseconds |
| 6 | 2 | Temperature | signed, hundredths of a °C |
| 8 | 1 | Mode (bits 7–4), flags (bits 3–0) | unsigned |

### 1. Get wiregen

Download the archive for your platform from the
[latest release](https://github.com/gatkinso/skywire/releases/latest) and
unpack it. On Linux x86_64:

```sh
VERSION=0.10.0
curl -LO https://github.com/gatkinso/skywire/releases/download/v$VERSION/skywire-$VERSION-linux-x86_64.tar.gz
tar xzf skywire-$VERSION-linux-x86_64.tar.gz
SKYWIRE=$PWD/skywire-$VERSION-linux-x86_64
```

The archives are `linux-x86_64`, `linux-aarch64`, `macos-arm64` and
`windows-x64` (a `.zip`); see [Installing](docs/install.md) for Homebrew and the
rest.

### 2. Write the specification

`SensorReading.json` says the same as the table:

```json
{
  "stringValues": { "classname": "SensorReading", "bitorder": "msb" },
  "members": {
    "1": { "type": "uint16_t", "name": "sensor_id" },
    "2": { "type": "uint32_t", "name": "timestamp_ms" },
    "3": { "type": "int16_t",  "name": "temp_centi_c" },
    "4": { "type": "uint8_t",  "name": "mode",  "bits": 4 },
    "5": { "type": "uint8_t",  "name": "flags", "bits": 4 }
  }
}
```

Members go on the wire in tag order (`"1"`, `"2"`, ...). Big-endian is the
default; a little-endian member would add `"endian": 1`. `bits` packs `mode`
and `flags` into one byte, and `"bitorder": "msb"` puts the first one in the
high bits, as the ICD draws it. [Specifications](docs/specs.md) has the rest.

### 3. Generate C

```sh
mkdir -p out
$SKYWIRE/bin/wiregen --language c --input_file SensorReading.json --outdir out --write_to_file
```

This writes `out/SensorReading.h` and `out/SensorReading.c`: a
`SensorReading` struct and the functions that read and write it.

### 4. Use it

`main.c` decodes a reading as the sensor sends it, then encodes it again:

```c
#include <stdio.h>
#include <string.h>
#include "SensorReading.h"

int main(void)
{
    /* A reading as the sensor sends it. */
    const char wire[] = "\x00\x2a" "\x00\x01\xe2\x40" "\x09\xc4" "\x31";
    SensorReading r;
    SensorReading_DeserializeSw(&r, wire);
    printf("sensor %u at %u ms: %.2f C, mode %u, flags %u\n",
           r.sensor_id, r.timestamp_ms, r.temp_centi_c / 100.0, r.mode, r.flags);

    /* And back to bytes. */
    char out[sizeof wire];
    SensorReading_SerializeSw(&r, out);
    printf("%zu bytes, %s\n", SensorReading_Size(),
           memcmp(out, wire, SensorReading_Size()) == 0 ? "same" : "different");
    return 0;
}
```

```sh
cc -std=c99 -I$SKYWIRE/include/skywire -Iout -o reading main.c out/SensorReading.c
./reading
```

```
sensor 42 at 123456 ms: 25.00 C, mode 3, flags 1
9 bytes, same
```

The runtime (`cstream.h`) is header-only, so there is nothing to link.

The same specification with `--language` `c++`, `csharp`, `go`, `java`,
`javascript`, `python3`, `rust` or `swift` writes the same 9 bytes.

## Installing

- **Release archives** for Linux (x86_64, arm64), macOS (Apple silicon) and
  Windows (x64): unpack and run, nothing else needed. See
  [Installing](docs/install.md).
- **Homebrew** on Apple silicon: `brew install gatkinso/tap/skywire`.
- **vcpkg and Conan**: a port and a recipe in [`packaging/`](packaging).
- **Swift package**: the Swift runtime, from this repository's `Package.swift`.
- **From source**: CMake 3.28, a C++17 compiler and protobuf; see
  [Building](docs/building.md).

## Documentation

| Page | What's in it |
|---|---|
| [Quick start](docs/quick-start.md) | The example above, step by step |
| [Why skywire?](docs/why.md) | What it's for, what it isn't, and the alternatives |
| [Specifications](docs/specs.md) | The JSON format: members, types, byte order, bitfields, nested messages, editor support |
| [Running wiregen](docs/cli.md) | The command line |
| [Generated code](docs/generated-code.md) | What wiregen writes, and serializing in place |
| [Languages](docs/languages.md) | Each language's runtime, tests and details |
| [Protobuf (PZ) mode](docs/protobuf.md) | Generated C++ speaking protobuf |
| [Lang files](docs/lang-files.md) | Customizing the generated code |
| [Installing](docs/install.md) | Release archives, Homebrew, vcpkg, Conan, versioning |
| [Using skywire](docs/using.md) | The CMake package, `skywire_generate()`, pkg-config, SwiftPM |
| [Building](docs/building.md) | Building skywire from source |
| [Testing](docs/testing.md) | The test suite, coverage, benchmarks and examples |
| [License](docs/license.md) | Skywire's license, and the generated code's |

## License

Skywire is licensed under the Apache License 2.0 (see [LICENSE](LICENSE)).
The code wiregen generates is yours: use it under any license, with no notice
or attribution required, and the runtime files it needs carry an exception
that grants the same freedom (see [License](docs/license.md)).

## Credits

This software is dedicated to all of the former racing Greyhounds who didn't
find a home after their racing "career" ended.  If you are considering getting 
a dog, please consider rescuing a Grey.  They are awesome pets.

In the U.S. visit: http://www.ngap.org/index.html

Skywire is offered under the Apache 2.0 license.
https://www.apache.org/licenses/LICENSE-2.0
Copyright 2014-2026 Geoffrey Atkinson.

Skywire began at Skyloupe, LLC.

Skywire uses and bundles the following third party software packages:

Google Protocol Buffers - (Apache 2.0)
https://developers.google.com/protocol-buffers/

Google Test - (Apache 2.0)
https://github.com/google/googletest
