#
#  Copyright (C) 2017 Geoffrey Atkinson
#
#  Licensed under the Apache License, Version 2.0 (the "License").
#  You may not use this file except in compliance with the License.
#  A copy of the License is located at
#
#  https://www.apache.org/licenses/LICENSE-2.0.html
#
#  or in the "license" file accompanying this file. This file is distributed
#  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#  express or implied. See the License for the specific language governing
#  permissions and limitations under the License.
#

cmake_minimum_required(VERSION 3.28)

# CMake 4.1+: normalize paths in file(GET_RUNTIME_DEPENDENCIES), which the
# Windows install of wiregen's DLLs uses; without it every install warns.
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

project(skywire VERSION 0.11.0)

include(TestBigEndian)
include (CTest)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

check_cxx_compiler_flag(-Wno-unused-value HAS_NO_UNUSED_VALUE)

if (HAS_NO_UNUSED_VALUE)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-value>)
endif()

# Build every C and C++ target (wiregen, the runtimes and all tests) with
# AddressSanitizer and UndefinedBehaviorSanitizer. Use a separate build
# directory, e.g. cmake -DSKYWIRE_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug.
option(SKYWIRE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(SKYWIRE_SANITIZE)
    if(MSVC)
        # MSVC supports AddressSanitizer only.
        add_compile_options(/fsanitize=address)
    else()
        set(SKYWIRE_SANITIZE_FLAGS
            -fsanitize=address,undefined
            -fno-sanitize-recover=all
            -fno-omit-frame-pointer
        )
        add_compile_options(${SKYWIRE_SANITIZE_FLAGS})
        add_link_options(${SKYWIRE_SANITIZE_FLAGS})
    endif()
    message(STATUS "Sanitizers enabled")
endif()

# Build every C and C++ target with gcov instrumentation (GCC or Clang), so
# ctest leaves .gcda files for scripts/coverage.py to report on. Use a
# separate Debug build directory, e.g.
# cmake -DSKYWIRE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug.
option(SKYWIRE_COVERAGE "Build with gcov coverage instrumentation (GCC or Clang)" OFF)

if(SKYWIRE_COVERAGE)
    if(MSVC)
        message(FATAL_ERROR "SKYWIRE_COVERAGE needs GCC or Clang")
    endif()
    add_compile_options(--coverage -O0)
    add_link_options(--coverage)
    message(STATUS "Coverage enabled")
endif()

# Build the targets in src/test/fuzz as libFuzzer fuzzers, with every C and
# C++ target instrumented for coverage, AddressSanitizer and
# UndefinedBehaviorSanitizer. Needs a Clang that ships libFuzzer (on macOS,
# Homebrew's llvm, not Apple's). Without it, the same targets build as plain
# programs that replay their corpora in ctest. See src/test/fuzz/README.md.
option(SKYWIRE_FUZZ "Build the fuzz targets with libFuzzer (Clang only)" OFF)

if(SKYWIRE_FUZZ)
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=fuzzer)
    check_cxx_source_compiles(
        "#include <cstddef>
         #include <cstdint>
         extern \"C\" int LLVMFuzzerTestOneInput(const uint8_t*, size_t) { return 0; }"
        SKYWIRE_HAVE_LIBFUZZER)
    unset(CMAKE_REQUIRED_LINK_OPTIONS)
    if(NOT SKYWIRE_HAVE_LIBFUZZER)
        message(FATAL_ERROR "SKYWIRE_FUZZ needs a Clang with libFuzzer (-fsanitize=fuzzer). "
                            "On macOS use Homebrew's: -DCMAKE_C_COMPILER=$(brew --prefix llvm)/bin/clang "
                            "-DCMAKE_CXX_COMPILER=$(brew --prefix llvm)/bin/clang++")
    endif()
    set(SKYWIRE_FUZZ_FLAGS
        -fsanitize=fuzzer-no-link,address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )
    add_compile_options(${SKYWIRE_FUZZ_FLAGS})
    add_link_options(${SKYWIRE_FUZZ_FLAGS})
    message(STATUS "Fuzzing enabled")
endif()

# Build the benchmarks in bench/ (not part of ctest). Use a Release build
# directory without sanitizers; see bench/README.md.
option(SKYWIRE_BENCH "Build the generated-code benchmarks in bench/" OFF)

# Build the example programs in examples/ and test them with ctest
# (skywire_example:<Lang>); see examples/README.md.
option(SKYWIRE_EXAMPLES "Build and test the example programs in examples/" ON)

# The revision wiregen --banner reports. Empty means git describe of the
# source tree; set it when building from a tarball or for a package.
set(SKYWIRE_REVISION "" CACHE STRING "Revision to report instead of git describe")

# For packages: vcpkg wants wiregen in tools/<port>, and on Windows installs
# the DLLs it needs from their own packages.
set(SKYWIRE_BIN_INSTALL_DIR "" CACHE STRING "Where to install wiregen (default: CMAKE_INSTALL_BINDIR)")
option(SKYWIRE_INSTALL_DLLS "Windows: install the DLLs wiregen needs beside it" ON)
if(SKYWIRE_BIN_INSTALL_DIR STREQUAL "")
    set(SKYWIRE_BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})
endif()

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
    add_definitions(-DSW_BIG_ENDIAN)
else()
    add_definitions(-DSW_LITTLE_ENDIAN)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

foreach(path ${CMAKE_PREFIX_PATH})
    message("Prefix Path: " ${path})
endforeach(path)

if(MSVC)
    # Set compiler options.
    set(configs
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )

    foreach(config ${configs})
        if(${config} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${config} "${${config}}")
        endif()
    endforeach()
endif(MSVC)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    # Distribution packages (Debian, Ubuntu) ship protobuf without its CMake
    # package; CMake's own FindProtobuf defines the same imported targets.
    find_package(Protobuf REQUIRED)
endif()
include_directories(${Protobuf_INCLUDE_DIRS})

# The tests need GoogleTest; a build with -DBUILD_TESTING=OFF (from CTest)
# builds wiregen without it, as the release builds do.
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

set(PROTOZERO_INC_DIR ${PROTOZERO_DIR}/include)

if(UNIX)
    add_definitions(-DAPPLE)
endif(UNIX)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif(UNIX AND NOT APPLE)

add_subdirectory(src)

if(SKYWIRE_BENCH)
    add_subdirectory(bench)
endif()

if(SKYWIRE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install wiregen, the runtimes and a CMake package, so another project can
# find_package(skywire CONFIG) and use skywire::wiregen, skywire::c_runtime,
# skywire::cpp_runtime and skywire_generate(). Nothing from the tests is
# installed.
set(SKYWIRE_INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/skywire)
set(SKYWIRE_RUNTIME_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/skywire/runtime)
set(SKYWIRE_LANG_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/skywire/lang)
set(SKYWIRE_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/skywire)

install(FILES lang/c/cstream.h
              lang/c/sw_endian.h
              lang/c/ctext.h
              lang/c/cbson.h
              lang/cpp/stream.h
              lang/cpp/streamable.h
              lang/cpp/text.h
              lang/cpp/bson.h
        DESTINATION ${SKYWIRE_INCLUDE_INSTALL_DIR})
install(FILES lang/swift/SWStream.swift lang/swift/SkywireText.swift DESTINATION ${SKYWIRE_RUNTIME_INSTALL_DIR}/swift)
install(FILES schema/skywire-spec.schema.json
              schema/skywire-lang.schema.json
        DESTINATION ${CMAKE_INSTALL_DATADIR}/skywire/schema)

install(EXPORT skywireTargets
        NAMESPACE skywire::
        DESTINATION ${SKYWIRE_CMAKE_INSTALL_DIR})
configure_package_config_file(cmake/skywireConfig.cmake.in
        ${PROJECT_BINARY_DIR}/skywireConfig.cmake
        INSTALL_DESTINATION ${SKYWIRE_CMAKE_INSTALL_DIR}
        PATH_VARS SKYWIRE_INCLUDE_INSTALL_DIR SKYWIRE_RUNTIME_INSTALL_DIR SKYWIRE_LANG_INSTALL_DIR)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/skywireConfigVersion.cmake
        COMPATIBILITY SameMinorVersion)
install(FILES ${PROJECT_BINARY_DIR}/skywireConfig.cmake
              ${PROJECT_BINARY_DIR}/skywireConfigVersion.cmake
              cmake/skywireGenerate.cmake
        DESTINATION ${SKYWIRE_CMAKE_INSTALL_DIR})

# skywire.pc, for builds without CMake. Its prefix is relative to the .pc
# file, so it is right wherever the install goes (cmake --install --prefix).
file(RELATIVE_PATH SKYWIRE_PC_PREFIX
     /prefix/${CMAKE_INSTALL_LIBDIR}/pkgconfig /prefix)
string(REGEX REPLACE "/$" "" SKYWIRE_PC_PREFIX "${SKYWIRE_PC_PREFIX}")
configure_file(cmake/skywire.pc.in ${PROJECT_BINARY_DIR}/skywire.pc @ONLY)
install(FILES ${PROJECT_BINARY_DIR}/skywire.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(FILES LICENSE README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
