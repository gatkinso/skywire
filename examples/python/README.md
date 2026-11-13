# Python example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written; then does the same with its JSON, XML and BSON forms
(`telemetry.json`, `telemetry.xml` and `telemetry.bson` beside the file; see
[JSON, XML and BSON](../../docs/text-formats.md)). See
[../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language python --write_to_file \
    --generate_json --generate_xml --generate_bson
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language python --write_to_file \
    --generate_json --generate_xml --generate_bson
cp examples/python/main.py out/
python3 out/main.py out/telemetry.bin
```

`main.py` imports `Position` and `Telemetry`, so it runs from the directory
they were generated into. Python 3, standard library only.

## What the build needs

- **Include directories:** Python's import path: `main.py` does `from Telemetry
  import Telemetry`, and `Telemetry.py` does `from Position import Position`,
  so the generated modules must be in the script's directory (as here) or on
  `PYTHONPATH`, e.g. `PYTHONPATH=out python3 examples/python/main.py
  out/telemetry.bin`.
- **Compile and link:** Nothing: the messages use only the standard library
  (`struct`), and `skywire_text.py`, the JSON, XML and BSON helpers, which
  wiregen writes beside them.
- **Lang file:** `--language python` reads `lang.python3.json` from the
  directory `wiregen` is in; the build copies it there from `lang/python/`. To
  use the source copy, or a customized one, pass `--lang_file
  lang/python/lang.python3.json` instead of `--language`. The lang file sets
  the generated file names.

`ctest -R skywire_example:Python` runs the same steps and checks the files are
`examples/Telemetry.bin`, `.json`, `.xml` and `.bson`, which every language
writes.
