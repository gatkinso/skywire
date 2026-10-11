# Cross-language serial test

`skywire_ring:Serial` passes one serialized message through every language's
generated code in turn, in a random order each run:

```
RingyMcRingface.bin -> C++ -> hop1.bin -> Go -> hop2.bin -> ... -> Python -> hop6.bin
```

```
ctest -R skywire_ring -V
```

## What is checked

At every hop, the language's generated **relay** program (`wiregen` writes it
when a spec names `outputrelayfilename` and tests are generated):

1. Checks the input is exactly `size()` bytes. A byte short or a byte extra fails.
2. Deserializes it and checks that every byte was consumed.
3. Checks every member against the shared test values (`test_values.cpp`) and
   names any member that differs. The relay also checks `Equals()`.
4. Re-serializes the message and checks the bytes are identical to its input.
5. Writes the bytes to the next hop's file.

The driver (`ring_serial.cmake`) also checks that each hop's output file is
byte-for-byte identical to the original input. Each hop therefore starts from
verified input, so a failure belongs to the language at that hop. Check 3 is
what catches a symmetric bug, such as a member read and written in the wrong
byte order: that bug round-trips its own bytes perfectly.

Before the chain runs, every relay must reject three damaged copies of the
input: one byte inverted (a random offset each run), one byte short, and one
byte extra. This proves the checks can fail.

## Controlling the order

| Environment | Effect |
|---|---|
| `SKYWIRE_RING_SEED=<n>` | Repeat an earlier run's order and inverted byte. Each run prints its seed. |
| `SKYWIRE_RING_ORDER=cpp,c,java,swift,go,python` | Run exactly this order. |
| `SKYWIRE_RING_REQUIRE_ALL=1` | Fail, instead of skipping, a language whose toolchain is missing. |

The C and C++ relays build with the project. The Swift, Go and Java relays are
compiled when the test runs. A missing toolchain prints `RING_LANG_SKIPPED`,
and fewer than two languages reports the test as Skipped.

## The message

`RingyMcRingface.json.in` is the one member list. CMake instantiates it per
language into `<build>/ring/specs/<lang>.json`. Tags are declared out of order
and differ in digit count, so wire order must follow the numeric tag.
Native endianness is left out because it is host-dependent.

`RingyMcRingface.bin` (110 bytes) is written by
[`scripts/make_ring_bin.py`](../../../../scripts/README.md), using Python's
`struct` and independently of every generator. The script prints this table:

| Offset | Member | Bytes |
|---:|---|---|
| 0 | `u8` uint8_t 201 | `c9` |
| 1 | `i8` int8_t -50 | `ce` |
| 2 | `ch` char 65 | `41` |
| 3 | `uch` unsigned char 200 | `c8` |
| 4 | `u16_be` 60000 | `ea 60` |
| 6 | `u16_le` 60000 | `60 ea` |
| 8 | `i16_be` -12345 | `cf c7` |
| 10 | `i16_le` -12345 | `c7 cf` |
| 12 | `u32_be` 3000000000 | `b2 d0 5e 00` |
| 16 | `u32_le` 3000000000 | `00 5e d0 b2` |
| 20 | `i32_be` -2000000000 | `88 ca 6c 00` |
| 24 | `i32_le` -2000000000 | `00 6c ca 88` |
| 28 | `u64_be` 12000000000 | `00 00 00 02 cb 41 78 00` |
| 36 | `u64_le` 12000000000 | `00 78 41 cb 02 00 00 00` |
| 44 | `i64_be` -9000000000 | `ff ff ff fd e7 8e e6 00` |
| 52 | `i64_le` -9000000000 | `00 e6 8e e7 fd ff ff ff` |
| 60 | `f32_be` 3.5 | `40 60 00 00` |
| 64 | `f32_le` 3.5 | `00 00 60 40` |
| 68 | `f64_be` 3.14159 | `40 09 21 f9 f0 1b 86 6e` |
| 76 | `f64_le` 3.14159 | `6e 86 1b f0 f9 21 09 40` |
| 84 | `str_padded` [16] "skywire" | `73 6b 79 77 69 72 65` + 9 × `00` |
| 100 | `str_exact` [7] "skywire" | `73 6b 79 77 69 72 65` (no terminator) |
| 107 | `str_truncated` [3] "sky" | `73 6b 79` |

## Changing the message

After you add members or types, or change a test value, regenerate the file
with `python3 scripts/make_ring_bin.py`, update the table above, and commit both.
`scripts/README.md` gives the steps for adding a type. To check the file is
current without writing it, run `python3 scripts/make_ring_bin.py --check`.

Each test value is written in two places on purpose:
`src/lib/skywire/test_values.cpp` sets the values the relays check against, and
the `TYPES` table in `make_ring_bin.py` sets the values written to the `.bin`.
A new type needs the **same value** in both. The duplication keeps the `.bin`
independent of wiregen: if wiregen's encoder and the file came from the same
code, a bug in that code would make them agree on the wrong bytes. If the two
values disagree, or the `.bin` is not regenerated, the serial test fails at its
first step. The relay reports either `input is N bytes, size() is M` or the
name of the member whose value differs.
