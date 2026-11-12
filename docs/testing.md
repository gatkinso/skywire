# Testing

## Running the tests

The tests are managed with CTest. After a successful build, run them from the
build directory:

    cd build_dir
    ctest -j8 --output-on-failure

With a multi-config generator (Visual Studio, Xcode) give the configuration
you built: `ctest -C Release`. Without it every test fails as "Not Run" (Missing
"-C <config>"). `ctest -R <regex>` runs a subset, for example
`ctest -R skywire_example`, and `ctest -N` lists the tests without running them.

## Test groups

Every test name starts with its group:

| Group | What it tests |
|---|---|
| `skywire_unit_test:*` | wiregen's own code: specifications, lang files and each generator (Google Test) |
| `wiregen_cli:*` | wiregen's command line: options, languages, lang files, errors |
| `qwikpirate_test:*` | The qwikpirate command-line parser (Google Test) |
| `skywire_generated_tests:*` | Generated C and C++, compiled into the build with their generated Google Test tests, plus bitfield, nested-message and PZ (protobuf) decoding checks |
| `skywire_generated_tests:*RoundTrip` | The generated Swift, Go, Python, Java, Rust, C# and JavaScript code, each with its generated unit tests |
| `skywire_generated_tests:JavaScriptTypes` | The generated TypeScript declarations, checked with tsc |
| `skywire_generated_tests:Proto3Wire` | The generated `.proto`, checked with protoc |
| `skywire_generated_tests:BsonInterop`, `BsonInteropC` | The generated C++ and C BSON against pymongo's BSON library, both ways |
| `skywire_text_diff:<language>` | A language's generated JSON, XML and BSON code against the C++, on 30,000 inputs |
| `skywire_golden:<language>`, `skywire_golden:<language>_text` | wiregen's output, without and with the JSON, XML and BSON flags, byte for byte, against the files in `src/test/golden` |
| `skywire_ring:Serial` | One message through every language in turn |
| `skywire_example:*` | The [example programs](#examples); only with `SKYWIRE_EXAMPLES` |
| `skywire_install:Consumer` | The install and its CMake package (see [Using skywire](using.md)) |
| `skywire_install:SwiftPM` | The Swift example as a package that depends on skywire's `Package.swift` |
| `skywire_schema:Check` | The [JSON Schemas](specs.md#editor-support) against every spec and lang file, and against wiregen's own checks |

The `skywire_generated_tests:*RoundTrip` tests generate each language's code
with its unit tests, then compile and run them. They need that language's
toolchain (swiftc; go 1.21 or newer; python3; a JDK; rustc 1.58 or newer; the
.NET SDK; Node.js 18 or newer) and
report Skipped without it, as do the matching `skywire_example` tests.
`JavaScriptTypes` needs `tsc` 5.1 or newer on `PATH` (`npm install -g
typescript`); it never downloads one.
`RustRoundTrip` also finds a rustup install in
`~/.cargo/bin` when it isn't on `PATH`, builds with plain `rustc` (no Cargo),
and fails on any compiler warning, on output that isn't rustfmt-clean (when
rustfmt is installed), and on clippy warnings (when clippy is installed).
`CSharpRoundTrip` builds every generated class and test into one program at C#
7.3, with warnings as errors and overflow checking on, and runs it twice: with
the `Span<T>` fast path, and with `SKYWIRE_PORTABLE` defined.
`JavaScriptRoundTrip` checks every generated file with `node --check` and runs
the tests with `node --test`; `JavaScriptTypes` type-checks, with `--strict`,
code that reads and writes every member and calls every method through the
generated `.d.ts` files, and expects an error from each wrongly typed use.

`skywire_ring:Serial` passes one serialized message through the generated code
of every language (C, C++, Swift, Go, Java, Python, Rust and JavaScript), in a random order each
run, ending back in the language it started in. Each step checks the message for
corruption or loss. A language without its toolchain is left out of the ring
(set `SKYWIRE_RING_REQUIRE_ALL=1` to fail instead), and the test reports
Skipped with fewer than two languages. Its input file is
written by `scripts/make_ring_bin.py`. See [`src/test/generated_tests/ring/README.md`](https://github.com/gatkinso/skywire/blob/master/src/test/generated_tests/ring/README.md)
and [`scripts/README.md`](https://github.com/gatkinso/skywire/blob/master/scripts/README.md).

`skywire_example:<Language>` builds and runs the [example programs](#examples).

`skywire_install:Consumer` installs the build into a scratch prefix (under
`build_dir/src/test/install/run`, never `CMAKE_INSTALL_PREFIX`), checks what
was installed, and builds and runs the C and C++ examples from a separate
project that uses the install (see [Using skywire](using.md)), with the CMake package and with
`skywire.pc`. It also checks that a project requiring the `cpp` component
without protozero fails with a clear message.

`skywire_generated_tests:BsonInterop` (C++) and `BsonInteropC` (C) have
pymongo decode the generated BSON of a message with a member of every type, at the shared test values and
at their limits (Decimal128, binary strings, NaN, -0), check every value and
type, and encode it back to the same bytes; then the generated code reads
documents pymongo wrote, with pymongo's choice of types and unknown elements
of every type, and rejects out-of-range ones. It needs Python's `pymongo`
module (`pip install pymongo`, or `python3-pymongo` on Debian and Ubuntu)
and report Skipped without it. CI runs it on Linux.

`skywire_golden:<language>` generates `ClassyMcClassface` (a member of every
type) and `Telemetry` (bitfields, a string, a nested message) with their tests
and relays, and no optional output such as `--generate_json`, and compares
every file with `src/test/golden/<language>`. It catches any change to code
users already generate, down to a blank line. `skywire_golden:<language>_text`
does the same with `--generate_json --generate_xml --generate_bson`, for each
language that has them (`c_text`, `cpp_text`, `csharp_text`, `go_text`, `java_text`, `javascript_text`, `python3_text`, `rust_text`, `swift_text`), so work on one language's text and
BSON code cannot quietly change another's. When a change is meant, build the
`skywire_update_golden` target, which rewrites `src/test/golden` from the
current wiregen, and commit the result with the change.

`skywire_text_diff:<language>` (in `src/test/text_diff`) holds each
language's JSON, XML and BSON code to the C++'s: it generates the ring
message in the language, reads every file of the fuzz corpora and 30,000
seeded mutations of them as JSON, XML and BSON, and fails unless the
language accepts exactly the inputs the C++ does and writes back exactly the
same bytes. The C++ side is `oracle.cpp`; each language has a small driver.
It needs Python 3, which makes the inputs, and the language's toolchain (Go
for `skywire_text_diff:go`, the .NET SDK for `skywire_text_diff:csharp`, rustc for
`skywire_text_diff:rust`, a JDK for `skywire_text_diff:java`, swiftc for
`skywire_text_diff:swift`, Node for `skywire_text_diff:javascript`), and reports Skipped without them.
The C code is held to the C++ by the `fuzz_c_cpp_text` fuzz target instead.

`skywire_schema:Check` runs `scripts/check_schemas.py`. It needs Python's
`jsonschema` module (`pip install jsonschema`, or `python3-jsonschema` on
Debian and Ubuntu) and reports Skipped without it. CI runs it on Linux.

## Coverage

To see what the tests cover, build with `SKYWIRE_COVERAGE` in its own
directory, run ctest, then report with `scripts/coverage.py` (needs
[gcovr](https://gcovr.com)):

    cmake -S skywire -B build-cov -DCMAKE_BUILD_TYPE=Debug -DSKYWIRE_COVERAGE=ON <your usual options>
    cmake --build build-cov && ctest --test-dir build-cov -j8
    python3 skywire/scripts/coverage.py build-cov --html build-cov/coverage/index.html

It counts wiregen (`src/lib`, `src/bin`, `include`) and the C and C++
runtimes (`lang/c`, `lang/cpp`); tests, examples and generated code are left
out. Run it on a build with every language toolchain, since the round trips
are what exercise each generator. CI's coverage job does this with GCC and
uploads the report.

## CDash

To submit results to a CDash dashboard (`ctest -D Experimental`), put your
own `CTestConfig.cmake` in the source directory. It is not part of the
repository.

## Benchmarks

`bench/` times the generated code of every language on the ring message (135
bytes, a different byte order on nearly every field, a nested message,
bitfields and fixed-length strings). The benchmarks are not part of ctest.
Build them in a Release directory without sanitizers:

	cmake -S skywire -B build-bench -DCMAKE_BUILD_TYPE=Release -DSKYWIRE_BENCH=ON <your usual options>
	cmake --build build-bench --target bench

or, after a build, `bench/run.sh --build build-bench [c cpp swift go python java rust csharp javascript]`.
[`bench/README.md`](https://github.com/gatkinso/skywire/blob/master/bench/README.md) says exactly what each number times and how to run them on
Windows.

Nanoseconds per message on an Apple M1 Pro (October 2026; Apple clang, Swift
6.4 with `-wmo`, Go 1.21, Python 3.14, JDK 19, Rust 1.99, .NET 10, Node 26), serializing into a
reused buffer:

| Language | Serialize | Deserialize | Notes |
|---|---:|---:|---|
| C++ | 7.8 | 11.9 | |
| C | 7.9 | 10.3 | |
| Go | 8.8 | 8.6 | `Serialize()` to a new slice: 33 |
| Rust | 10.0 | 8.1 | `serialize()` to a new `Vec`: 26 |
| C# | 20 | 20 | `Serialize()` to a new `byte[]`: 28 |
| Java | 30 | 31 | `serialize()` to a new `byte[]`: 35 |
| Swift | 34 | 43 | |
| JavaScript | 53 | 121 | `serialize()` to a new `Uint8Array`: 255 |
| Python | 1096 | 852 | 368 / 446 for a message in one byte order |

The deserialize times for C, C++ and Swift include copying the input into the
stream. Expect Swift to be slower without `-wmo` (whole-module optimization,
the default for SwiftPM and Xcode release builds).

wiregen's own time grows linearly with message size: on the M1 Pro a
4000-member message, with its tests and relay, generates in 0.03 to 0.5 s
depending on the language (a few times that on older or virtual machines).
`bench/wiregen_scale.py` measures it on your machine (see `bench/README.md`).

## Examples

`examples/` has a small program for each language: C, C++, Swift, Go, Python,
Java, Rust, C# and JavaScript. Each one fills a message that holds a nested message, writes it
to a file, reads it back and checks the copy is equal. A further C++ example,
`examples/protobuf`, sends the message through a PZ-mode stream to protoc's C++
code for the generated `.proto` and back. Both specs are in
`examples/specs/`, and each language's directory has its own README with the
wiregen and compiler commands. The examples are built and tested by ctest
(`skywire_example:<Language>`); configure with `-DSKYWIRE_EXAMPLES=OFF` to
leave them out. See [`examples/README.md`](https://github.com/gatkinso/skywire/blob/master/examples/README.md).

## Fuzzing

The fuzz targets, their corpora and how to run them are in
[`src/test/fuzz/README.md`](https://github.com/gatkinso/skywire/blob/master/src/test/fuzz/README.md).
