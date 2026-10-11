# Scripts

| Script | Purpose |
|---|---|
| `make_ring_bin.py` | Writes `RingyMcRingface.bin`, the input file of the cross-language serial test. |
| `test_wiregen.bat` | Windows: runs wiregen over the `cfg/*.test.json` specs. |

## make_ring_bin.py

The cross-language serial test (`skywire_ring:Serial`, see
[`src/test/generated_tests/ring/README.md`](../src/test/generated_tests/ring/README.md))
starts from a checked-in serialized message, `RingyMcRingface.bin`. This script
writes that file from the ring spec `RingyMcRingface.json`.

The bytes come from Python's `struct` module, not from wiregen or any generated
code. That keeps the file an independent reference: a bug in wiregen's own
encoder can't make the file and the generated code agree on the wrong bytes.

```
python3 scripts/make_ring_bin.py            # write the file, print its layout
python3 scripts/make_ring_bin.py --check    # exit 1 if the file is out of date
```

It reads the members from the spec, in numeric tag order, with each member's
byte order and string length. It prints a Markdown table of offsets and bytes
in the same format as the table in the ring README, so you can paste it in.

Options:

| Option | Default |
|---|---|
| `--spec <file>` | `src/test/generated_tests/ring/RingyMcRingface.json` |
| `--out <file>` | `src/test/generated_tests/ring/RingyMcRingface.bin` |
| `--check` | Compare with `--out` instead of writing it. |

The script exits with an error if a member's type has no entry in its `TYPES`
table, or if a member uses `endian: 2` (native), which has no fixed wire bytes.

### Adding a member or a type

A new member of a type the script already knows:

1. Add the member to `RingyMcRingface.json`.
2. Run `make_ring_bin.py`, check the printed offsets, update the table in the
   ring README, and commit the new `.bin`.

A new type:

1. Add its test value to `src/lib/skywire/test_values.cpp`.
2. Add a row to `TYPES` in `make_ring_bin.py`: the `struct` format code and the
   **same value** as in step 1.
3. Add members using it to `RingyMcRingface.json`, preferably one big-endian
   (`endian: 0`) and one little-endian (`endian: 1`).
4. Run `make_ring_bin.py`, check the printed offsets, update the table in the
   ring README, and commit the new `.bin`.

The values are written in two places on purpose. If they disagree, or the
`.bin` is not regenerated, the serial test fails at its first step. The
relay reports either `input is N bytes, size() is M` or the name of the
member whose value differs.
