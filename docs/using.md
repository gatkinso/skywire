# Using skywire

How a project builds generated code against an installed skywire: a
[release archive](install.md), a `cmake --install` of your own
[build](building.md), Homebrew, vcpkg or Conan.

## CMake package

An installed skywire is a CMake package, used much like protobuf's:

    find_package(skywire CONFIG REQUIRED COMPONENTS c)

    add_executable(app main.c)
    skywire_generate(TARGET app LANGUAGE c SPECS specs/Position.json specs/Telemetry.json)

Point `CMAKE_PREFIX_PATH` at the install prefix, and for C++ at protozero too
(`PROTOZERO_DIR`, or vcpkg):

    cmake -S app -B app_build -DCMAKE_PREFIX_PATH=$HOME/skywire \
          -DPROTOZERO_DIR=$HOME/apps/protozero

The components are `c` and `cpp`; without `COMPONENTS` you get whichever are
available. Generated C++ always needs protozero, a header-only library that is
not part of the install. The `cpp` component looks for it on the include path,
under `PROTOZERO_DIR`, or in `SKYWIRE_PROTOZERO_INCLUDE_DIR` (the directory
holding `protozero/`). Without it, `skywire::cpp_runtime` is not defined, a
`find_package` that requires `cpp` fails and says so, and
`skywire_generate(TARGET ... LANGUAGE cpp)` stops with the same advice.

The package provides:

| Name | What it is |
|---|---|
| `skywire::wiregen` | The generator, for your own `add_custom_command` |
| `skywire::c_runtime` | The header-only runtime for generated C (`cstream.h`) |
| `skywire::cpp_runtime` | The header-only runtime for generated C++, with protozero's include directory and C++17 |
| `skywire_generate()` | Generates C or C++ at build time |
| `SKYWIRE_INCLUDE_DIR`, `SKYWIRE_RUNTIME_DIR`, `SKYWIRE_LANG_DIR` | The installed headers, the Swift runtime and the lang files |

    skywire_generate(LANGUAGE <c|cpp>
                     SPECS <spec.json>...
                     [TARGET <target>]
                     [OUT_DIR <dir>]
                     [OUT_VAR <variable>]
                     [NAMESPACE <namespace>])

`skywire_generate()` runs wiregen on each specification whenever it (or
another in `SPECS`) changes. Its output goes to `OUT_DIR`, by default
`skywire_c` or `skywire_cpp` in the current binary directory. With `TARGET` the
generated sources are added to that target, which also gets `OUT_DIR` on its
include path and links the language's runtime. `OUT_VAR` receives the generated
file names. Give a nested message's specification in the same call as the
message that holds it, in the same directory. For the other languages, run
`skywire::wiregen` yourself; Swift code also needs
`${SKYWIRE_RUNTIME_DIR}/swift/SWStream.swift`, or the [Swift package](#swift-package).

## Without CMake

Without CMake, use `skywire.pc`. It gives the include path, and its `wiregen`,
`langdir` and `runtimedir` variables give the generator, the lang files and the
Swift runtime. Its paths are relative to the `.pc` file, so they stay right
wherever the install is:

    export PKG_CONFIG_PATH=$HOME/skywire/lib/pkgconfig
    WIREGEN=$(pkg-config --variable=wiregen skywire)
    mkdir -p out
    $WIREGEN --language c --input_file Position.json --outdir out --write_to_file
    cc -std=c99 $(pkg-config --cflags skywire) -Iout -o app main.c out/Position.c

Neither runtime has anything to compile or link. For C++, add protozero's
include directory and `-std=c++17`; protozero has no `.pc` of its own.

## Swift package

A Swift package can depend on skywire for SWStream instead of copying
`SWStream.swift`. The `Package.swift` at the top of the repository has one
library, `Skywire`:

```swift
dependencies: [
    .package(url: "https://github.com/gatkinso/skywire.git", from: "0.10.0"),
],
targets: [
    .executableTarget(name: "app", dependencies: [.product(name: "Skywire", package: "skywire")]),
]
```

Generated Swift starts with `#if canImport(Skywire)` / `import Skywire`, so the
same files build with the package or with a copy of `SWStream.swift` in the
module. Your own files that use `SWStream` directly need `import Skywire`.
wiregen itself still runs outside SwiftPM; check its output in, or generate it
before `swift build`.

## Generated tests

Code generated with `--generate_tests` needs Google Test, for C and C++ only;
the tests for the other languages use their standard library. The C and C++
test files include `<gtest/gtest.h>` and `<gmock/gmock.h>` and have no
`main()`, so link them with `GTest::gtest_main` and `GTest::gmock` (from
`find_package(GTest)`). Swift, Go, Python, Java, Rust, C# and JavaScript tests
need only their own toolchain (for JavaScript, Node's built-in `node:test`).

`src/test/install/consumer` is a small project of this kind; ctest builds it
against a scratch install (`skywire_install:Consumer`), once through the CMake
package and once, when pkg-config is there, through `skywire.pc`.
