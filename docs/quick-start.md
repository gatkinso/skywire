# Quick start

Say a temperature sensor's interface control document (ICD) describes the
reading it sends, 9 bytes, little-endian:

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 2 | Sensor ID | unsigned |
| 2 | 4 | Timestamp | unsigned, milliseconds |
| 6 | 2 | Temperature | signed, hundredths of a °C |
| 8 | 1 | Mode (bits 7–4), flags (bits 3–0) | unsigned |

## 1. Get wiregen

Download the archive for your platform from the
[latest release](https://github.com/gatkinso/skywire/releases/latest) and
unpack it. On Linux x86_64:

```sh
VERSION=0.11.0
curl -LO https://github.com/gatkinso/skywire/releases/download/v$VERSION/skywire-$VERSION-linux-x86_64.tar.gz
tar xzf skywire-$VERSION-linux-x86_64.tar.gz
SKYWIRE=$PWD/skywire-$VERSION-linux-x86_64
```

The archives are `linux-x86_64`, `linux-aarch64`, `macos-arm64` and
`windows-x64` (a `.zip`); see [Installing](install.md) for Homebrew and the
rest.

## 2. Write the specification

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

Members go on the wire in tag order (`"1"`, `"2"`, ...). Little-endian is the
default; a big-endian member would add `"endian": 1`. `bits` packs `mode`
and `flags` into one byte, and `"bitorder": "msb"` puts the first one in the
high bits, as the ICD draws it. [Specifications](specs.md) has the rest.

## 3. Generate C

```sh
mkdir -p out
$SKYWIRE/bin/wiregen --language c --input_file SensorReading.json --outdir out --write_to_file
```

This writes `out/SensorReading.h` and `out/SensorReading.c`: a
`SensorReading` struct and the functions that read and write it.

## 4. Use it

`main.c` decodes a reading as the sensor sends it, then encodes it again:

```c
#include <stdio.h>
#include <string.h>
#include "SensorReading.h"

int main(void)
{
    /* A reading as the sensor sends it. */
    const char wire[] = "\x2a\x00" "\x40\xe2\x01\x00" "\xc4\x09" "\x31";
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

## 5. JSON, XML and BSON

Add `--generate_json` to the wiregen command (or `--generate_xml`,
`--generate_bson`, or all three), and the reading has its text form too:

```c
char json[128];
SensorReading_ToJson(&r, json, sizeof json);   /* returns the length, like snprintf */
puts(json);
SensorReading_FromJson(&r, json, strlen(json)); /* 0, or -1 if it is not one */
```

```
{"sensor_id":42,"timestamp_ms":123456,"temp_centi_c":2500,"mode":3,"flags":1}
```

Every language writes exactly that, and reads it back; see
[JSON, XML and BSON](text-formats.md).

## Next

- Another language: the same specification with `--language` `c++`, `csharp`,
  `go`, `java`, `javascript`, `python3`, `rust` or `swift`. Each writes the
  same 9 bytes; see [Languages](languages.md).
- In a CMake project, `skywire_generate()` runs wiregen at build time; see
  [Using skywire](using.md).
- Point your editor at the [JSON Schema](specs.md#editor-support) to get
  completion and checking as you write specifications.
- Not sure skywire fits? See [Why skywire?](why.md).
