# Conan

`conanfile.py` is a Conan 2 recipe that builds wiregen from the source tree it
is in. The package holds:

| Path | Contents |
|---|---|
| `bin/` | `wiregen`, with protobuf linked in statically |
| `include/skywire/` | The C and C++ runtime headers |
| `lib/cmake/skywire/` | skywire's own CMake package: `find_package(skywire)` and `skywire_generate()` |
| `lib/pkgconfig/skywire.pc` | For builds without CMake |
| `share/skywire/` | The lang files and the Swift runtime |

It requires protobuf (ConanCenter's 6.33.5, static, and not visible to
consumers) and protozero (1.7.1, whose headers are, since generated C++
includes them). The version comes from `project()` in `CMakeLists.txt`. It
needs C++17: Conan's detected MSVC profile says `compiler.cppstd=14`, so pass
`-s compiler.cppstd=17` there.

CMakeDeps doesn't write a config for skywire (`cmake_find_mode` is `none`):
consumers get the package's own `skywireConfig.cmake`, which `CMakeToolchain`
puts on `CMAKE_PREFIX_PATH`, because it also defines `skywire_generate()`.

## Using it

```
conan create packaging/conan --build=missing
```

puts `skywire/<version>` in the local cache. A project then requires it:

```python
def requirements(self):
    self.requires("skywire/0.11.0")
```

and uses `find_package(skywire CONFIG REQUIRED)` and `skywire_generate()` as
with any other install ([docs/using.md](../../docs/using.md)). Cross-compiling isn't supported yet:
`skywire::wiregen` is the package's own wiregen, built for the target.

## Testing

`conan create` also builds `test_package/`, which builds
`src/test/install/consumer` against the package (CMake package,
`skywire_generate()`, and `skywire.pc` when pkg-config is there), runs each
program and checks it writes `examples/Telemetry.bin`. The release workflow
runs it on its Linux x86_64 build.

The first run builds protobuf from source unless ConanCenter has a binary for
your profile, which takes several minutes; after that it is in the cache.

## ConanCenter

`conan-center/` is a draft recipe for
[conan-center-index](https://github.com/conan-io/conan-center-index), laid
out as `recipes/skywire/` is there. It has not been submitted. It differs
from the recipe above:

- `source()` fetches the release tarball named in `conandata.yml` (URL and
  sha256), and the version comes from the reference.
- Conan writes the CMake config. The package keeps `skywire_generate()` as a
  build module, `conan-skywire-generate.cmake`, which also defines the
  `skywire::wiregen` target and the `SKYWIRE_*` variables skywire's own
  config file would. The runtimes are the components `skywire::c_runtime`
  and `skywire::cpp_runtime`.
- protoc comes from a `tool_requires` on protobuf, so the build can run it
  whatever the host protobuf is.
- protobuf, abseil and zlib must be static (the recipe refuses shared ones):
  they are private requirements, so they are not on a consumer's library
  path when `skywire_generate()` runs wiregen.

To try it (any version that has been released):

```sh
cd packaging/conan/conan-center/all
conan create . --version 0.11.0 --build=missing
```

protobuf 7 needs C++17. Conan's detected profile on Windows (MSVC) is C++14,
so add `-s:a compiler.cppstd=17` there; `-s:a` sets it for the build profile
too, which builds the protoc that runs during the build.

It has passed Release and Debug builds on macOS, Ubuntu 24.04 (GCC 13) and
Rocky Linux 10 (GCC 14), and a Release build on Windows 11 (MSVC 19.50). It
needs CMake 3.28, which it gets as a `tool_requires`: ConanCenter's build
machines have older CMake.

For each release, add the version to `config.yml` and `conandata.yml`, with
the sha256 of `https://github.com/gatkinso/skywire/archive/refs/tags/vX.Y.Z.tar.gz`.
