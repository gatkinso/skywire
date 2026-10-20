# Swift example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written. See [../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language swift --write_to_file
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language swift --write_to_file
swiftc -o out/example_swift lang/swift/SWStream.swift \
    out/Position.swift out/Telemetry.swift examples/swift/main.swift
out/example_swift out/telemetry.bin
```

Compile the runtime, `lang/swift/SWStream.swift`, in the same module as the
messages. Add `-O -wmo` for an optimized build.

## What the build needs

- **Include directories:** None: Swift has no headers. The lang file makes each
  message `import Foundation`.
- **Compile and link:** One module: the runtime `lang/swift/SWStream.swift`,
  the generated `Position.swift` and `Telemetry.swift`, and `main.swift`, all
  passed to one `swiftc` command. The messages read and write through the
  runtime's `SWStream`, so compile it with them. Nothing else to link.
- **Lang file:** `--language swift` reads `lang.swift.json` from the directory
  `wiregen` is in; the build copies it there from `lang/swift/`. To use the
  source copy, or a customized one, pass `--lang_file
  lang/swift/lang.swift.json` instead of `--language`. The lang file sets the
  generated file names and the runtime includes each message starts with.

`ctest -R skywire_example:Swift` runs the same steps and checks the file is
`examples/Telemetry.bin`.
