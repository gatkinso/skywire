# Installing

| Way | What you get |
|---|---|
| [Release archive](#release-archives) | Prebuilt wiregen, runtimes and CMake package for Linux, macOS and Windows |
| [Homebrew](#homebrew) | The macOS archive, plus protozero |
| [vcpkg](#vcpkg-and-conan) or [Conan](#vcpkg-and-conan) | wiregen built from source as a dependency of your project |
| [Swift package](using.md#swift-package) | The Swift runtime only |
| [From source](building.md) | Everything, including the tests |

## Release archives

Each [release](https://github.com/gatkinso/skywire/releases) has a prebuilt
archive per platform, so you can use wiregen without building skywire:

| Archive | Runs on |
|---|---|
| `skywire-<version>-linux-x86_64.tar.gz` | Linux x86_64 with glibc 2.35 or newer (Ubuntu 22.04, Debian 12, RHEL 10 and later) |
| `skywire-<version>-linux-aarch64.tar.gz` | The same, on arm64 |
| `skywire-<version>-macos-arm64.tar.gz` | macOS 12 or newer on Apple silicon |
| `skywire-<version>-windows-x64.zip` | Windows 10 or newer, x64 |

wiregen in these archives needs nothing else installed: protobuf, abseil and
(on Linux) the C++ runtime are linked in. Each archive is an install prefix,
the same as `cmake --install` makes (see [Using skywire](using.md)):

| Path | Contents |
|---|---|
| `bin/` | `wiregen` |
| `include/skywire/` | The C and C++ runtime headers |
| `share/skywire/` | The lang files, the Swift runtime and the [JSON Schemas](specs.md#editor-support) |
| `lib/cmake/skywire/`, `lib/pkgconfig/` | The CMake package and `skywire.pc` |

Unpack it anywhere and run `bin/wiregen`, or point `CMAKE_PREFIX_PATH` at it.
`SHA256SUMS` lists each archive's checksum:

    sha256sum --check --ignore-missing SHA256SUMS

and each archive has a build provenance attestation, which shows it was built
by this repository's release workflow from the tagged commit:

    gh attestation verify skywire-<version>-linux-x86_64.tar.gz --repo gatkinso/skywire

The macOS build is not notarized. If macOS refuses to run a wiregen downloaded
with a browser, remove the quarantine attribute:
`xattr -d com.apple.quarantine bin/wiregen`.

## Homebrew

On Apple silicon Macs, Homebrew installs the same archive, with protozero
for generated C++:

    brew install gatkinso/tap/skywire

The formula and its tests are in [`packaging/homebrew`](https://github.com/gatkinso/skywire/blob/master/packaging/homebrew/README.md).

## vcpkg and Conan

A vcpkg overlay port and a Conan 2 recipe, which build wiregen from source,
are in [`packaging/vcpkg`](https://github.com/gatkinso/skywire/blob/master/packaging/vcpkg/README.md) and
[`packaging/conan`](https://github.com/gatkinso/skywire/blob/master/packaging/conan/README.md). Both give your
project the same CMake package as an install (see [Using skywire](using.md)).
Neither is in its central registry yet.

## Versioning

Skywire follows [Semantic Versioning](https://semver.org/). The version covers:

* the specification JSON format,
* the lang file format,
* the public API of the generated code and of the runtime files, and
* the bytes a generated message writes (its wire format).

A change that breaks any of these needs a new major version. Before 1.0, a
minor version may break them too; `CHANGELOG.md` lists every such change under
**Changed** or **Removed**. `wiregen --version` prints the version, and
`wiregen --banner` the commit it was built from.
