# The macOS release build: vcpkg's community arm64-osx-release (static
# libraries, release only) built for macOS 12 and later, like wiregen itself.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET 12.0)

set(VCPKG_BUILD_TYPE release)
