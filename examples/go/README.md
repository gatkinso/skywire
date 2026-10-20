# Go example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written. See [../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language go --write_to_file --namespace main
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language go --write_to_file --namespace main
cp examples/go/main.go out/
printf 'module wiregen/example\n\ngo 1.21\n' > out/go.mod
(cd out && go build -o example_go .)
out/example_go out/telemetry.bin
```

The example is a command, so `--namespace main` puts the messages in package
`main`. Go 1.21 or newer.

## What the build needs

- **Include directories:** None: the generated files and `main.go` are one
  package (`main`) in one directory, `out`, with the `go.mod` written above.
  The messages use only the standard library.
- **Compile and link:** `go build` in `out` builds every `.go` file there. No
  runtime file and no module to fetch.
- **Lang file:** `--language go` reads `lang.go.json` from the directory
  `wiregen` is in; the build copies it there from `lang/go/`. To use the source
  copy, or a customized one, pass `--lang_file lang/go/lang.go.json` instead of
  `--language`. The lang file sets the generated file names.

`ctest -R skywire_example:Go` runs the same steps and checks the file is
`examples/Telemetry.bin`.
