# vcpkg

`skywire/` is an overlay port. It builds wiregen from the source tree it is
in and installs:

| Path in the vcpkg prefix | Contents |
|---|---|
| `tools/skywire/` | `wiregen` (and on Windows, with a dynamic triplet, the DLLs it needs) |
| `include/skywire/` | The C and C++ runtime headers |
| `share/skywire/` | The CMake package, the lang files, the Swift runtime and `copyright` |
| `lib/pkgconfig/skywire.pc` | For builds without CMake |

It depends on protobuf (wiregen links it) and protozero (generated C++
includes it). Only a release build is made: wiregen is a tool, and the
runtimes are headers.

## Using it

Until the port is in vcpkg's registry, point vcpkg at this directory. In a
project's `vcpkg.json`:

```json
{
  "dependencies": ["skywire"]
}
```

and configure with the overlay:

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_OVERLAY_PORTS=/path/to/skywire/packaging/vcpkg
```

Then `find_package(skywire CONFIG REQUIRED)` and `skywire_generate()` work
as they do with any other install ([docs/using.md](../../docs/using.md)). Cross-compiling isn't
supported yet: `skywire::wiregen` is the target triplet's wiregen.

## Testing

```
python3 packaging/vcpkg/test_port.py --vcpkg-root ~/apps/vcpkg
```

It installs the port in manifest mode into a scratch directory, builds
`src/test/install/consumer` with it, runs each program against
`examples/Telemetry.bin`, and checks the layout above. The vcpkg checkout must
contain the baseline in `.github/vcpkg/vcpkg.json`. The first run builds
protobuf, which takes several minutes; after that vcpkg's binary cache has it.
`--triplet` and `--host-triplet` pick the triplets, and `--keep` keeps the
scratch directory.

The port's `version` must match `project()` in `CMakeLists.txt`; the script
checks. The release workflow runs it on its Linux x86_64 build job.

## Submitting upstream

Once the repository is public, a port for microsoft/vcpkg replaces the
`SOURCE_PATH` line with `vcpkg_from_github` for the release tag, plus a
second `vcpkg_from_github` for `gatkinso/qwikpirate` (a submodule, which
tarballs don't include) copied into `3rd_party/qwikpirate`, and adds the
version to vcpkg's version database (`vcpkg x-add-version skywire`).
