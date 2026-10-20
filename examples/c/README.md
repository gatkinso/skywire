# C example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written. See [../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language c --write_to_file
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language c --write_to_file
cc -std=c99 -Iout -Ilang/c -o out/example_c \
    examples/c/main.c out/Position.c out/Telemetry.c lang/c/cstream.c
out/example_c out/telemetry.bin
```

The generated `Telemetry.c` and `Position.c` are compiled with the C runtime,
`lang/c/cstream.c`. Setters take pointers to their values; `Telemetry_Equals`
returns 1 for equal messages.

## What the build needs

- **Include directories:** `out` (the generated `Telemetry.h`, `Position.h`)
  and `lang/c` (`cstream.h`, which `Telemetry.h` includes because the lang file
  lists it, and `sw_endian.h`, which `cstream.h` includes).
- **Compile and link:** `main.c`, the generated `Position.c` and `Telemetry.c`,
  and the runtime `lang/c/cstream.c`. Nothing else: no library to link.
- **Lang file:** `--language c` reads `lang.c.json` from the directory
  `wiregen` is in; the build copies it there from `lang/c/`. To use the source
  copy, or a customized one, pass `--lang_file lang/c/lang.c.json` instead of
  `--language`. The lang file sets the generated file names and the runtime
  includes each message starts with.

`ctest -R skywire_example:C` runs the same steps and checks the file is
`examples/Telemetry.bin`.
