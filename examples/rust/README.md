# Rust example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written; then does the same with its JSON, XML and BSON forms
(`telemetry.json`, `telemetry.xml` and `telemetry.bson` beside the file; see
[JSON, XML and BSON](../../docs/text-formats.md)). See
[../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language rust --write_to_file \
    --generate_json --generate_xml --generate_bson
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language rust --write_to_file \
    --generate_json --generate_xml --generate_bson
cp examples/rust/main.rs out/
rustc --edition 2021 -o out/example_rust out/main.rs
out/example_rust out/telemetry.bin
```

`main.rs` is the crate root and declares the generated modules, `position` and
`telemetry`, and `skywire_text`, the JSON, XML and BSON helpers wiregen writes
beside them, so it is compiled from the directory they are in. Plain rustc 1.58
or newer, no Cargo.

## What the build needs

- **Include directories:** None: `main.rs` declares `mod position;` and `mod
  telemetry;`, and rustc finds `position.rs` and `telemetry.rs` next to it, so
  the crate root goes in `out` with them. `telemetry.rs` reaches `Position` as
  `super::position`, so both must be modules of the same parent.
- **Compile and link:** Only the crate root, `out/main.rs`; rustc follows its
  `mod` lines, `mod skywire_text;` included. The messages use only `std`: no
  runtime crate and no Cargo.
- **Lang file:** `--language rust` reads `lang.rust.json` from the directory
  `wiregen` is in; the build copies it there from `lang/rust/`. To use the
  source copy, or a customized one, pass `--lang_file lang/rust/lang.rust.json`
  instead of `--language`. The lang file sets the generated file names.

`ctest -R skywire_example:Rust` runs the same steps and checks the files are
`examples/Telemetry.bin`, `.json`, `.xml` and `.bson`, which every language
writes.
