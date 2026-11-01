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
    self.requires("skywire/0.9.0")
```

and uses `find_package(skywire CONFIG REQUIRED)` and `skywire_generate()` as
with any other install (README §3.2). Cross-compiling isn't supported yet:
`skywire::wiregen` is the package's own wiregen, built for the target.

## Testing

`conan create` also builds `test_package/`, which builds
`src/test/install/consumer` against the package (CMake package,
`skywire_generate()`, and `skywire.pc` when pkg-config is there), runs each
program and checks it writes `examples/Telemetry.bin`. The release workflow
runs it on its Linux x86_64 build.

The first run builds protobuf from source unless ConanCenter has a binary for
your profile, which takes several minutes; after that it is in the cache.

## Submitting upstream

Once the repository is public, a recipe for conan-center-index fetches the
release tarball in `source()` (from a `conandata.yml` with its URL and
sha256) instead of `export_sources()`, takes qwikpirate (a submodule, which
tarballs don't include) the same way, sets the version from the reference
rather than `set_version()`, and has its own `test_package`.
