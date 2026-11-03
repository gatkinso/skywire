# Building skywire

You only need to build skywire to work on it, or on a platform with no
[release archive](install.md).

## Getting the source

The source is at https://github.com/gatkinso/skywire.
Qwikpirate (command-line parsing) is a git submodule, so clone with

    git clone --recursive https://github.com/gatkinso/skywire.git

or run `git submodule update --init` in an existing clone.

## Dependencies

| Package | Used for |
|---|---|
| Google Protocol Buffers | wiregen reads specifications through it; the proto3 tests run its `protoc` |
| protozero | The C++ runtime: every generated C++ message has protobuf (PZ) functions, so any program with generated C++ needs its headers, as do skywire's tests and examples |
| Google Test | Skywire's tests, and the C and C++ tests that `--generate_tests` writes |
| Qwikpirate | wiregen's command line (bundled as a submodule) |

## Requirements

* CMake 3.28 or newer
* A C++17 compiler (and a C99 compiler)

Skywire is built and tested on:

| Platform | Compiler | CMake generators |
|---|---|---|
| macOS (Apple silicon) | Apple clang | Unix Makefiles |
| Ubuntu 24.04 (x86_64) | GCC | Unix Makefiles |
| Windows 11 (x64) | Visual Studio 2026 (MSVC 19.50) | Ninja, Visual Studio 18 2026 |

The tests that compile generated code in other languages need that language's
toolchain, and are skipped without it (see [Testing](testing.md)).

Every release is built and tested by GitHub Actions before it is published
(`.github/workflows/ci.yml`, called from `release.yml`): Ubuntu 24.04 with GCC
and every language toolchain (no test may be skipped), Ubuntu with Clang under
AddressSanitizer and UndefinedBehaviorSanitizer, and macOS 15 with Apple clang
and Windows with Visual Studio 2026, both with every language toolchain (again,
no test may be skipped). CI can also be run by hand on any branch, as can the
libFuzzer workflow (`.github/workflows/fuzz.yml`); see
[`src/test/fuzz`](https://github.com/gatkinso/skywire/blob/master/src/test/fuzz/README.md). Pull requests run no workflows.

## Building

Skywire builds with CMake only.

Point CMake at your protobuf and Google Test installs with `CMAKE_PREFIX_PATH`,
and at protozero with `PROTOZERO_DIR` (the directory holding its `include`):

    cmake -S skywire -B build_dir -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_PREFIX_PATH="$HOME/apps/protobuf;$HOME/apps/googletest" \
          -DPROTOZERO_DIR=$HOME/apps/protozero \
          -DCMAKE_INSTALL_PREFIX=$HOME/skywire
    cmake --build build_dir
    cmake --install build_dir

On Ubuntu (24.04 or newer) the distribution's packages are enough:

    sudo apt install libprotobuf-dev protobuf-compiler libgtest-dev libgmock-dev libprotozero-dev
    cmake -S skywire -B build_dir -DCMAKE_BUILD_TYPE=Release -DPROTOZERO_DIR=/usr

On macOS, Homebrew's `protobuf`, `googletest` and `protozero` work the same
way, with `-DCMAKE_PREFIX_PATH=$(brew --prefix)` and
`-DPROTOZERO_DIR=$(brew --prefix protozero)`.

To build only wiregen and what it installs, configure with
`-DBUILD_TESTING=OFF -DSKYWIRE_EXAMPLES=OFF`. Then protobuf is the only
dependency: Google Test and protozero are needed for the tests and examples.

`cmake --install build_dir --prefix <dir>` installs somewhere else without
reconfiguring. Under the prefix:

| Path | Contents |
|---|---|
| `bin/` | `wiregen` (on Windows, with the protobuf and abseil DLLs it needs) |
| `include/skywire/` | The C and C++ runtimes, both header-only: `cstream.h`, `sw_endian.h`, `stream.h`, `streamable.h` |
| `share/skywire/runtime/` | The Swift runtime, `swift/SWStream.swift` |
| `share/skywire/lang/` | The lang files, `lang.*.json` |
| `share/skywire/schema/` | The [JSON Schemas](specs.md#editor-support) for specifications and lang files |
| `lib/cmake/skywire/` | The CMake package (see [Using skywire](using.md)). Some Linux systems use `lib64` or a multiarch directory in place of `lib`. |
| `lib/pkgconfig/` | `skywire.pc`, for builds without CMake (see [Using skywire](using.md#without-cmake)) |
| `share/doc/skywire/` | `README.md` and `LICENSE` |

Nothing from the tests is installed. The generated C#, Go, Java, JavaScript,
Python and Rust code needs no runtime. The build also leaves a ready-to-run `wiregen`, with its
lang files beside it, in `build_dir/bin`.

Build options:

| Option | Default | Effect |
|---|---|---|
| `SKYWIRE_EXAMPLES` | ON | Build and test the programs in `examples/` (see [Examples](testing.md#examples)). |
| `SKYWIRE_BENCH` | OFF | Build the benchmarks in `bench/` (see [Benchmarks](testing.md#benchmarks)). |
| `SKYWIRE_SANITIZE` | OFF | Build all C and C++ with AddressSanitizer and UndefinedBehaviorSanitizer (MSVC: AddressSanitizer only). Use a separate build directory. |
| `SKYWIRE_FUZZ` | OFF | Build the fuzz targets in `src/test/fuzz` with libFuzzer, and everything with ASan and UBSan (Clang only; see [`src/test/fuzz`](https://github.com/gatkinso/skywire/blob/master/src/test/fuzz/README.md)). Use a separate build directory. |
| `SKYWIRE_COVERAGE` | OFF | Build all C and C++ with gcov coverage counters, for `scripts/coverage.py` (GCC or Clang; see [Coverage](testing.md#coverage)). Use a separate Debug build directory. |
| `SKYWIRE_BIN_INSTALL_DIR` | `bin` | Where `cmake --install` puts wiregen, relative to the prefix; wiregen finds the lang files from there. The vcpkg port uses `tools/skywire`. |
| `SKYWIRE_INSTALL_DLLS` | ON | Windows: install the DLLs wiregen needs beside it. The vcpkg port turns it off, since those DLLs come from their own packages. |

## Windows

What is it with Windows? Why must already complex tasks be further muddied
with needless complexity? Sigh... it is a mystery.

That said, it is a mystery that we must embrace if we are to target the OS that
runs 90% of the computers on the planet. The easiest route is vcpkg: install
`protobuf`, `gtest` and `protozero` for the `x64-windows` triplet, then, from a
Visual Studio x64 developer prompt:

    cmake -S skywire -B build_dir -G Ninja -DCMAKE_BUILD_TYPE=Release ^
          -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
          -DVCPKG_TARGET_TRIPLET=x64-windows
    cmake --build build_dir

or `-G "Visual Studio 18 2026" -A x64` in place of `-G Ninja`, building with
`cmake --build build_dir --config Release`. vcpkg puts protozero on the include
path, so `PROTOZERO_DIR` is not needed.

To install:

    cmake --install build_dir --prefix C:/skywire

adding `--config Release` for the Visual Studio generator. `bin` gets
`libprotobuf.dll` and `abseil_dll.dll` beside `wiregen.exe`, so the installed
wiregen runs without vcpkg on `PATH`.
