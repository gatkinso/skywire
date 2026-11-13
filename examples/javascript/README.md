# JavaScript example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written; then does the same with its JSON, XML and BSON forms
(`telemetry.json`, `telemetry.xml` and `telemetry.bson` beside the file; see
[JSON, XML and BSON](../../docs/text-formats.md)). See
[../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language javascript --write_to_file \
    --generate_json --generate_xml --generate_bson
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language javascript --write_to_file \
    --generate_json --generate_xml --generate_bson
cp examples/javascript/main.js out/
echo '{ "type": "module" }' > out/package.json
node out/main.js out/telemetry.bin
```

`main.js` imports `./Position.js` and `./Telemetry.js`, so it runs from the
directory they were generated into. Node.js 18 or later; the messages
themselves use only `Uint8Array`, `DataView`, `BigInt` and `TextEncoder`, so
they also run unchanged in browsers, Deno and Bun. Only `main.js` uses Node
(`node:fs`, to read and write the file).

## What the build needs

- **Include directories:** ES module imports: `main.js` does `import {
  Telemetry } from "./Telemetry.js"`, and `Telemetry.js` does `import {
  Position } from "./Position.js"`, so the generated modules must sit
  together, here next to `main.js`.
- **Compile and link:** Nothing. The `package.json` with `"type": "module"`
  makes Node 18 and 20 treat the `.js` files as ES modules (Node 22 and later
  detect that on their own). The messages import `skywire_text.js`, the JSON,
  XML and BSON helpers, which wiregen writes beside them. TypeScript users get
  `Telemetry.d.ts`, `Position.d.ts` and `skywire_text.d.ts`, generated
  alongside.
- **Lang file:** `--language javascript` (or `js`) reads
  `lang.javascript.json` from the directory `wiregen` is in; the build copies
  it there from `lang/javascript/`. To use the source copy, or a customized
  one (for example `.mjs` file names in a CommonJS package), pass `--lang_file
  lang/javascript/lang.javascript.json` instead of `--language`. The lang file
  sets the generated file names.

`ctest -R skywire_example:JavaScript` runs the same steps and checks the files are
`examples/Telemetry.bin`, `.json`, `.xml` and `.bson`, which every language
writes.
