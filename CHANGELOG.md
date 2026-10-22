# Changelog

All notable changes to skywire are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[Semantic Versioning](https://semver.org/). Before 1.0, a minor version may
break compatibility; such changes are listed under **Changed** or **Removed**.

## [0.9.0] - Unreleased

The first versioned release, covering the work since the `modernize` merge.
The date is set when v0.9.0 is tagged.

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
  sanitizers), macOS and Windows; pull requests also get a clang-format check
  of the changed lines.

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
  manifest mode, not only classic mode.
- `int8_t` members serialized as 4 bytes in C++ (#37).
- Endianness detection and C++ stream memory safety (#39).
- PZ tests and protobuf reserved field numbers (#41).

### Removed
- The unused `lang_file` field in specifications (#42).
- The obsolete `c++11` lang file (#48).

[0.9.0]: https://github.com/gatkinso/skywire/releases/tag/v0.9.0
