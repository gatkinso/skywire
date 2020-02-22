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

cmake_minimum_required(VERSION 3.5)

project(skywire)

include(TestBigEndian)
include (CTest)

TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
    add_definitions(-DSW_BIG_ENDIAN)
else()
    add_definitions(-DSW_LITTLE_ENDIAN)
endif()

set(CMAKE_CXX_STANDARD 11)
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

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

enable_testing()
find_package(GTest REQUIRED)

if(UNIX)
    add_definitions(-DAPPLE)
endif(UNIX)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DLINUX)
endif(UNIX AND NOT APPLE)

add_subdirectory(3rd_party/qwikpirate)
add_subdirectory(3rd_party/skytrans)
add_subdirectory(src)
