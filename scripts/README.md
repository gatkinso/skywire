# Scripts

| Script | Purpose |
|---|---|
| `make_ring_bin.py` | Writes `RingyMcRingface.bin`, the input file of the cross-language serial test. |
| `test_wiregen.bat` | Windows: runs wiregen over the `cfg/*.test.json` specs. |
| `check_ctest_skips.py` | CI: fails if ctest skipped a test (reads `ctest --output-junit`). |
| `release_notes.py` | Release: checks a tag against the source and prints its CHANGELOG section. |
| `release_smoke_test.py` | Release: checks an unpacked release archive works on a machine that didn't build it. |
| `run_fuzzers.py` | Runs the libFuzzer targets in a `-DSKYWIRE_FUZZ=ON` build for a while each. |
| `coverage.py` | Reports line and branch coverage of a `-DSKYWIRE_COVERAGE=ON` build after ctest. |

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
byte order and string length. Bitfield members are packed into storage units
by the script's own copy of the rules in `include/skywire/bitfields.h`
(including the spec's `bitorder`), with the values from its `bits_value()`,
which must match `test_values::bits_value`; each unit is one row of the table. A member whose type is another message is that
message's own encoding, inline, read from `<type>.json` beside the spec. It prints a Markdown table of offsets and bytes
in the same format as the table in the ring README, so you can paste it in.

Options:

| Option | Default |
|---|---|
| `--spec <file>` | `src/test/generated_tests/ring/RingyMcRingface.json` |
| `--out <file>` | `src/test/generated_tests/ring/RingyMcRingface.bin` |
| `--check` | Compare with `--out` instead of writing it. |

The script exits with an error if a member's type has no entry in its `TYPES`
table and no spec of its own, or if a member uses `endian: 2` (native), which has no fixed wire bytes.

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

## check_ctest_skips.py

The language round trips, the ring and the examples report Skipped when a
toolchain is missing. CI's all-languages job installs every toolchain, so a
skip there means its setup is broken; this script turns that into a failure.

```
ctest --test-dir build --output-junit ctest.xml
python3 scripts/check_ctest_skips.py build/ctest.xml               # no skips allowed
python3 scripts/check_ctest_skips.py build/ctest.xml --allow Swift # except tests matching the regex
```

## Making a release

`.github/workflows/release.yml` builds the archives (README 1.4), tests each
on a fresh runner and publishes a GitHub release when a `v*` tag is pushed.

1. Set the version in `project(skywire VERSION x.y.z)` in `CMakeLists.txt`
   and make sure `CHANGELOG.md` has a `## [x.y.z]` section.
2. Optionally try a pre-release first: tag `vx.y.z-rc1` and push the tag. It
   is published as a pre-release, with the same notes. Delete it when done
   (`gh release delete vx.y.z-rc1 --cleanup-tag`).
3. Date the CHANGELOG section, `## [x.y.z] - YYYY-MM-DD`, and merge that.
4. Tag that commit `vx.y.z` and push the tag.

The workflow refuses a tag whose version isn't the project version, and a
release (not a pre-release) whose CHANGELOG section isn't dated.

## release_notes.py

```
python3 scripts/release_notes.py v0.9.0       # the notes for 0.9.0
python3 scripts/release_notes.py v0.9.0-rc1   # a pre-release of 0.9.0: the same notes
```

Prints the `## [0.9.0]` section of `CHANGELOG.md`, or fails if the tag isn't
`v<major>.<minor>.<patch>[-<pre-release>]`, if `CMakeLists.txt` has a
different project version, or, for a release, if the section isn't dated.

## release_smoke_test.py

```
python3 scripts/release_smoke_test.py skywire-0.9.0-linux-x86_64 --version 0.9.0
```

Run on an unpacked archive. It checks that `wiregen --version` prints the
version; that wiregen needs only system libraries (`ldd`, `otool -L` or
`dumpbin /dependents`) and `bin/` holds nothing else; that it generates every
language from the archive's lang files; and that the generated C, compiled
against the archive's headers, writes `examples/Telemetry.bin`. It uses `$CC`
or `cc`, or `cl` on Windows, from a Visual Studio developer shell.

## run_fuzzers.py

```
cmake --build build-fuzz --target fuzzers
python3 scripts/run_fuzzers.py build-fuzz                       # every target, 60 s each
python3 scripts/run_fuzzers.py build-fuzz --seconds 1800 fuzz_spec
```

Runs each fuzz target (`src/test/fuzz`) from its seeds, as listed in
`build-fuzz/fuzz/<target>.fuzz`, plus the corpus it kept from earlier runs
in `build-fuzz/fuzz/work/<target>`. Targets run in parallel, one per CPU
(`--jobs`). It prints the runs, speed and coverage of each, and for a target
that crashes, times out, runs out of memory or fails a check, the error, the
failing input (in `build-fuzz/fuzz/artifacts/<target>/`) and how to reproduce
it; then it exits 1. The `Fuzz` workflow runs it in CI.

## coverage.py

```
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DSKYWIRE_COVERAGE=ON
cmake --build build-cov && ctest --test-dir build-cov -j8
python3 scripts/coverage.py build-cov                              # summary table
python3 scripts/coverage.py build-cov --html cov/index.html --xml cov/coverage.xml
python3 scripts/coverage.py build-cov --reset                      # zero the counters
```

Runs [gcovr](https://gcovr.com) over the build's counters (`.gcda` files) and
prints each file's line coverage with the lines never run, then the line,
function and branch totals. It counts `src/lib`, `src/bin`, `include`,
`lang/c` and `lang/cpp`, the code wiregen and the C/C++ runtimes are built
from, and leaves out tests, examples, benchmarks, `3rd_party` and generated
code. `--html` writes a page per file, `--xml` Cobertura XML for a coverage
service. It picks `gcov` for a GCC build and `llvm-cov gcov` for Clang
(through `xcrun` for Apple Clang). Branch totals differ between the two; CI
reports with GCC.

Counters add up over runs, including the wiregen runs during the build. To
measure one run on its own, `--reset`, then rebuild from clean and run ctest.
