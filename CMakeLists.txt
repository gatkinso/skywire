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
