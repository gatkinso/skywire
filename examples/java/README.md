# Java example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written. See [../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language java --write_to_file
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language java --write_to_file
javac -d out/classes out/Position.java out/Telemetry.java examples/java/Main.java
java -cp out/classes Main out/telemetry.bin
```

The messages are in package `telemetry`, the specs' namespace; `Main` imports
them.

## What the build needs

- **Include directories:** The class path: `javac -d out/classes` writes the
  messages under `out/classes/telemetry/` (their package is the specs'
  namespace), and `java -cp out/classes` finds both them and `Main` there.
- **Compile and link:** The generated `Position.java` and `Telemetry.java` and
  `Main.java`, in one `javac` command. They use only the JDK
  (`java.nio.ByteBuffer`); no runtime jar.
- **Lang file:** `--language java` reads `lang.java.json` from the directory
  `wiregen` is in; the build copies it there from `lang/java/`. To use the
  source copy, or a customized one, pass `--lang_file lang/java/lang.java.json`
  instead of `--language`. The lang file sets the generated file names.

`ctest -R skywire_example:Java` runs the same steps and checks the file is
`examples/Telemetry.bin`.
