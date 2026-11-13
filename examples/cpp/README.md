# C++ example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written; then does the same with its JSON, XML and BSON forms
(`telemetry.json`, `telemetry.xml` and `telemetry.bson` beside the file; see
[JSON, XML and BSON](../../docs/text-formats.md)). See
[../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language c++ --write_to_file \
    --generate_json --generate_xml --generate_bson
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language c++ --write_to_file \
    --generate_json --generate_xml --generate_bson
c++ -std=c++17 -Iout -Ilang/cpp -Ilang/c -I<protozero>/include -o out/example_cpp \
    examples/cpp/main.cpp
out/example_cpp out/telemetry.bin
```

The generated code is header-only, as is the runtime in `lang/cpp`. `stream.h`
includes protozero (for PZ mode), so add its include directory: the build's
`PROTOZERO_DIR`.

## What the build needs

- **Include directories:** `out` (the generated headers), `lang/cpp`
  (`streamable.h`, which the lang file makes every message include,
  `stream.h`, and `text.h` and `bson.h` for the JSON, XML and BSON forms), `lang/c` (`sw_endian.h`, shared with C) and protozero's
  `include` directory (`stream.h` includes `<protozero/pbf_writer.hpp>` for PZ
  mode).
- **Compile and link:** Only `main.cpp`: the messages, the runtime and
  protozero are all header-only. Nothing to link.
- **Lang file:** `--language c++` reads `lang.cpp.json` from the directory
  `wiregen` is in; the build copies it there from `lang/cpp/`. To use the
  source copy, or a customized one, pass `--lang_file lang/cpp/lang.cpp.json`
  instead of `--language`. The lang file sets the generated file names and the
  runtime includes each message starts with.

The same messages can also be sent as protobuf, through a PZ-mode stream: see
[../protobuf/README.md](../protobuf/README.md).

`ctest -R skywire_example:C++` runs the same steps and checks the files are
`examples/Telemetry.bin`, `.json`, `.xml` and `.bson`, which every language
writes.
