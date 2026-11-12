# Changelog

All notable changes to skywire are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/). Before 1.0, a minor version may
break compatibility; such changes are listed under **Changed** or **Removed**.

## [Unreleased]

### Added
- JSON and XML text forms for generated C++ messages: `--generate_json` adds
  `to_json` and `from_json`, `--generate_xml` adds `to_xml` and `from_xml`
  (and `skywire_generate()` takes `JSON` and `XML`). The format, which every
  language will write byte for byte, is in `docs/text-formats.md`. The
  runtime is the header-only, standard-library-only `lang/cpp/text.h`,
  installed with `stream.h`. Generated tests check the exact text, round
  trips and rejection of bad input; `fuzz_cpp_json` and `fuzz_cpp_xml` fuzz
  the readers. Without the options the generated code is unchanged; the other
  languages refuse them for now.
- BSON for generated C++ messages: `--generate_bson` adds `to_bson` and
  `from_bson` (and `skywire_generate()` takes `BSON`). Elements are in tag
  order, named after the members; integers are int32 or int64, a `uint64_t`
  above 2^63-1 a Decimal128, strings a string or (if not UTF-8) binary; the
  mapping is in `docs/text-formats.md`. The runtime is the header-only,
  standard-library-only `lang/cpp/bson.h`, installed with `stream.h`.
  Generated tests check the exact bytes, round trips and rejection of bad
  documents; `skywire_generated_tests:BsonInterop` checks the bytes against
  pymongo, and `fuzz_cpp_bson` fuzzes the reader.
- JSON, XML and BSON for generated C: `X_ToJson`/`X_FromJson`,
  `X_ToXml`/`X_FromXml` and `X_ToBson`/`X_FromBson`, with the same flags.
  The writers follow `snprintf` (they return the length needed); the readers
  return 0 or -1 and leave the message unchanged on failure. The runtime is
  the header-only C99 `lang/c/ctext.h` and `lang/c/cbson.h`, installed with
  `cstream.h`; BSON reading needs no allocation. Generated tests check the
  same expected bytes as C++, `skywire_generated_tests:BsonInteropC` checks
  the BSON against pymongo, and `fuzz_c_cpp_text` checks that C reads and
  writes exactly what C++ does.
- JSON, XML and BSON for generated Python: `to_json`/`from_json`,
  `to_xml`/`from_xml` and `to_bson`/`from_bson`, with the same flags. wiregen
  writes the helpers, `skywire_text.py` (standard library only), beside the
  generated modules; a lang file names it with the new `outputruntimefilename`.
  `skywire_text_diff:python3` checks that Python reads and writes exactly what
  C++ does, on the fuzz corpora and 30,000 mutations of them.
- JSON, XML and BSON for generated Go: `ToJSON`/`FromJSON` and `ToXML`/`FromXML`,
  which also implement `json.Marshaler`/`Unmarshaler` and
  `xml.Marshaler`/`Unmarshaler`, and `MarshalBSON`/`UnmarshalBSON` (the
  MongoDB Go driver's interfaces, with no dependency on it). wiregen writes
  the helpers, `skywire_text.go` (standard library only), into the message's
  package. `skywire_text_diff:go` checks Go against C++ as for Python.
- JSON, XML and BSON for generated C#: `ToJson`/`FromJson`, `ToXml`/`FromXml`
  and `ToBson`/`FromBson`, the readers returning false, with the message
  unchanged, on bad input. wiregen writes the helpers, `SkywireText.cs`
  (C# 7.3, base class library only, internal types in `Skywire.Text`), beside
  the classes; numbers are read and written with exact arithmetic, so .NET
  Framework, Unity and .NET agree. `skywire_text_diff:csharp` checks C#
  against C++ as for Python.
- JSON, XML and BSON for generated Rust: `to_json`/`from_json`,
  `to_xml`/`from_xml` and `to_bson`/`from_bson`, the readers returning a new
  message or an `InvalidData` `std::io::Error`. wiregen writes the helpers,
  the module `skywire_text.rs` (std only, rustc 1.58 or newer), beside the
  message modules. `skywire_text_diff:rust` checks Rust against C++ as for
  Python.
- Golden files: `skywire_golden:<language>` compares wiregen's output for
  every language, byte for byte, with `src/test/golden`, and
  `skywire_golden:c_text` and `cpp_text` the C and C++ output with the JSON, XML
  and BSON flags;
  `skywire_update_golden` rewrites them after a deliberate change.
- A C# generator: `--language csharp` (or `cs`, `c#`) writes a portable C# 7.3
  class per message, for .NET Framework 4.6.2+, Unity and .NET, with
  generated tests and the `skywire_generated_tests:CSharpRoundTrip` ctest
  (.NET SDK), an example (`examples/csharp`, `skywire_example:CSharp`) and a
  benchmark (`bench/csharp`). On .NET Core 2.1+ and .NET Standard 2.1+ it
  uses `Span<T>` and `BinaryPrimitives` (`SKYWIRE_PORTABLE` turns that off).
  No relay yet.
- A JavaScript generator: `--language javascript` (or `js`) writes an ES2020
  module per message, with TypeScript declarations (`.d.ts`), for Node 18+,
  browsers, Deno, Bun and bundlers: `Uint8Array`, `DataView` and `BigInt`
  (for the 64-bit integers) only, with no Node APIs and no runtime package.
  Generated `node:test` tests and the
  `skywire_generated_tests:JavaScriptRoundTrip` ctest (Node 18+),
  `skywire_generated_tests:JavaScriptTypes`, which type-checks the
  declarations (tsc 5.1+), a relay in the ring test, an example
  (`examples/javascript`, `skywire_example:JavaScript`) and a benchmark
  (`bench/javascript`).
- `--language golang`, an alias for `go`.
- JSON Schemas for specifications and lang files,
  `schema/skywire-spec.schema.json` and `schema/skywire-lang.schema.json`
  (draft 2020-12), installed in `share/skywire/schema/`. Editors that know
  the schema complete and check specifications as you type. The
  `skywire_schema:Check` ctest (needs Python's `jsonschema`) validates every
  spec and lang file in the repository and checks the schema rejects what
  wiregen rejects.
- wiregen accepts a top-level `"$schema"` in a specification or lang file and
  ignores it, so a file can name its schema.
- A `Package.swift` with one library, `Skywire` (the Swift runtime,
  SWStream), so a Swift package can depend on skywire instead of copying
  `SWStream.swift`, and the `skywire_install:SwiftPM` ctest that builds the
  Swift example that way.
- A Homebrew formula for Apple silicon Macs, `gatkinso/tap/skywire`, that
  installs the macOS release archive with protozero.
  `packaging/homebrew/make_formula.py` writes it from a release's
  `SHA256SUMS`, and the release workflow's macOS smoke test installs it from
  the archive it tests, runs `brew test`, and builds
  `src/test/install/consumer` against it.
- A vcpkg overlay port, `packaging/vcpkg/skywire`: wiregen in
  `tools/skywire`, the runtime headers, the CMake package and `skywire.pc`,
  depending on protobuf and protozero. `packaging/vcpkg/test_port.py`
  installs it in manifest mode and builds `src/test/install/consumer` with it;
  the release workflow runs it on its Linux x86_64 build.
- A Conan 2 recipe, `packaging/conan`: wiregen (protobuf linked in
  statically), the runtime headers, skywire's own CMake package and
  `skywire.pc`, requiring protozero. Its `test_package` builds
  `src/test/install/consumer`; the release workflow runs `conan create` on
  its Linux x86_64 build.
- CMake options `SKYWIRE_BIN_INSTALL_DIR` (where wiregen is installed; it
  finds the lang files relative to it) and `SKYWIRE_INSTALL_DLLS` (Windows:
  install wiregen's DLLs beside it, default ON), for packages.
- `bench/wiregen_scale.py`, which times wiregen itself on large (100 to 4000
  members) and deeply nested (up to 64 levels) synthetic messages in every
  language.
- Fuzz targets for the generated C and C++ decoders (SW and protobuf), a C
  versus C++ differential target, and wiregen's spec and lang file handling,
  with the `SKYWIRE_FUZZ` option for libFuzzer builds. Every build replays
  their corpora in ctest, and the `Fuzz` workflow runs them by hand
  (`src/test/fuzz`, `scripts/run_fuzzers.py`).
- Test coverage reports: the `SKYWIRE_COVERAGE` option builds with gcov
  counters and `scripts/coverage.py` reports on wiregen and the C/C++
  runtimes; CI's coverage job uploads the report. New tests for wiregen's
  string input mode (`--input_str`, `--langspec`), `--silent`, unreadable
  and unwritable files, and specification file writing.

### Changed
- The README is now a short introduction with a quick start; the reference
  material moved to `docs/`, which is also built as a site with MkDocs
  (`mkdocs.yml`) and checked by CI's new docs job. New pages: a quick start
  from an ICD table, "Why skywire?" (what skywire doesn't do, and Kaitai
  Struct, protobuf, FlatBuffers, Cap'n Proto, DFDL and hand-written structs
  compared), and a specification reference.
- Generated Swift files start with `#if canImport(Skywire)` / `import Skywire`
  (`banner_str` in `lang.swift.json`), so they build against the Swift package
  as well as a copied `SWStream.swift`.
- GitHub Actions run only when a release is cut: a `v*` tag runs CI alongside
  the release build and publishes only if both pass. Pull requests and pushes
  to master run nothing; CI and fuzzing can be started by hand on any branch.
  The pull-request clang-format check is gone (run `git clang-format` locally).
- Lang file tokens (the `*_substr` and `*_token` values) are literal text,
  no longer regular expressions. A custom lang file that relied on regex
  syntax in a token must spell the token out. wiregen no longer compiles a
  regex for every substitution: the fuzz targets run about 2.2 times faster
  and wiregen itself uses about 30% less CPU, with the generated code
  unchanged byte for byte.

### Fixed
- wiregen aborted with an uncaught exception when a lang file had a value
  the generators could not use, such as a malformed pattern or a Java type
  mapped to something unexpected. It now reports an invalid lang file
  (`SW_LANG_INVALID`). Found by fuzzing.
- wiregen ignored `--langspec` with `--input_file` and asked for a language.
  `--langspec` now works with either input mode; the library adds
  `process_file_langstr` (and `Processor::process_file_langstr`) for it.
- In string mode wiregen's summary named no input ("Successful process: ");
  it now says `--input_str`. With neither `--input_file` nor `--input_str`
  wiregen says so instead of failing on a blank spec, and a blank message
  string is no longer reported as a blank lang string.

### Removed
- The unused `cfg/*.test.json` specifications and `scripts/test_wiregen.bat`,
  which ran wiregen over them by hand; the ctest suite covers every language.

## [0.9.0] - 2026-10-25

The first versioned release, covering the work since the `modernize` merge.

### Added
- Generators for C, C++, Go, Java, Python 3, Rust and Swift, plus proto3
  `.proto` output, each with generated unit tests and a relay program
  (#35, #36, #38, #47).
- Fixed-length `string` members (`char[N]`), byte-for-byte compatible across
  languages (#35).
- Bitfield members (#46) and nested messages (#45).
- Protobuf (PZ) mode for generated C++, through protozero (#44, #41).
- The cross-language ring test: one message through every language in a
  random order (#40).
- Language-neutral specifications: the command line picks the language;
  `--language`, `--lang_file`, `--generate_relay` and `--version` (#42).
- Spec validation, short-read errors in C, C++ and Swift, and
  AddressSanitizer/UBSan builds (`SKYWIRE_SANITIZE`) (#39).
- Benchmarks for every language (`SKYWIRE_BENCH`) and performance work across
  the runtimes (#49).
- Example programs for every language (`SKYWIRE_EXAMPLES`) (#50).
- `cmake --install`, a CMake package with `skywire_generate()`, a pkg-config
  file, and a header-only C runtime (#51).
- `wiregen --banner` with version, revision and build information (#52).
- Windows builds with Visual Studio and Ninja (#43).
- `.clang-format`, `.editorconfig` and this changelog.
- GitHub Actions CI: Linux (GCC with every language, and Clang with
  sanitizers), and macOS and Windows (MSVC), each with every language; pull
  requests also get a clang-format check of the changed lines.
- Release archives for Linux (x86_64, aarch64), macOS (arm64) and Windows
  (x64), built by `.github/workflows/release.yml` from a `v*` tag, with a
  wiregen that needs no protobuf or other libraries installed, `SHA256SUMS`,
  and build provenance attestations. README sections on releases (1.4) and
  versioning (1.5).
- `-DBUILD_TESTING=OFF` builds wiregen without Google Test or protozero.
- `CHANGELOG.md` is installed with the README and LICENSE.

### Changed
- Copyright is held by Geoffrey Atkinson; "All rights reserved" is gone, and
  skywire is plainly Apache-2.0. The repository and the qwikpirate submodule
  live under `github.com/gatkinso`.
- The runtime files (`cstream.h`, `sw_endian.h`, `stream.h`, `streamable.h`,
  `SWStream.swift`) carry a runtime exception: programs using them with
  generated code may be distributed under any terms. Generated code carries no
  license obligation (README 1.3).
- Builds against distribution packages of protobuf (Ubuntu's ship no CMake
  package): CMake falls back to its own FindProtobuf.

### Fixed
- On Windows checkouts that convert line endings, the Go example failed
  gofmt; `.gitattributes` keeps Go and Rust sources LF.
- The install test (`skywire_install:Consumer`) now works with vcpkg in
  manifest mode, not only classic mode, and no longer fails when it finds a
  pkg-config that the consumer project doesn't.
- `int8_t` members serialized as 4 bytes in C++ (#37).
- Endianness detection and C++ stream memory safety (#39).
- PZ tests and protobuf reserved field numbers (#41).

### Removed
- The unused `lang_file` field in specifications (#42).
- The obsolete `c++11` lang file (#48).

[Unreleased]: https://github.com/gatkinso/skywire/compare/v0.9.0...HEAD
[0.9.0]: https://github.com/gatkinso/skywire/releases/tag/v0.9.0
