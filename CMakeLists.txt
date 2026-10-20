#
#  Copyright (C) 2017 Skyloupe, LLC
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

project(skywire VERSION 0.9.0)

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

# Build the benchmarks in bench/ (not part of ctest). Use a Release build
# directory without sanitizers; see bench/README.md.
option(SKYWIRE_BENCH "Build the generated-code benchmarks in bench/" OFF)

# Build the example programs in examples/ and test them with ctest
# (skywire_example:<Lang>); see examples/README.md.
option(SKYWIRE_EXAMPLES "Build and test the example programs in examples/" ON)

# The revision wiregen --banner reports. Empty means git describe of the
# source tree; set it when building from a tarball or for a package.
set(SKYWIRE_REVISION "" CACHE STRING "Revision to report instead of git describe")

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
find_package(Protobuf CONFIG REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

enable_testing()
find_package(GTest REQUIRED)

set(PROTOZERO_INC_DIR ${PROTOZERO_DIR}/include)

if(UNIX)
    add_definitions(-DAPPLE)
endif(UNIX)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif(UNIX AND NOT APPLE)

add_subdirectory(3rd_party/qwikpirate)
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
              lang/cpp/stream.h
              lang/cpp/streamable.h
        DESTINATION ${SKYWIRE_INCLUDE_INSTALL_DIR})
install(FILES lang/swift/SWStream.swift DESTINATION ${SKYWIRE_RUNTIME_INSTALL_DIR}/swift)

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

install(FILES LICENSE README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
