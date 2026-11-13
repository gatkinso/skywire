#
#  Copyright (C) 2026 Geoffrey Atkinson
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

#  A third-party project that uses an installed skywire: it builds the C and
#  C++ example programs from examples/ with skywire_generate() (with the
#  JSON, XML and BSON forms, which the programs use), and, when
#  pkg-config is there, the C one again with only skywire.pc. Run by
#  run_install_test.cmake with CMAKE_PREFIX_PATH set to the install.

cmake_minimum_required(VERSION 3.28)

project(skywire_consumer C CXX)

find_package(skywire CONFIG REQUIRED COMPONENTS c cpp)

set(specs ${EXAMPLES_DIR}/specs/Position.json ${EXAMPLES_DIR}/specs/Telemetry.json)

add_executable(consumer_c ${EXAMPLES_DIR}/c/main.c)
skywire_generate(TARGET consumer_c LANGUAGE c SPECS ${specs} JSON XML BSON)

add_executable(consumer_cpp ${EXAMPLES_DIR}/cpp/main.cpp)
skywire_generate(TARGET consumer_cpp LANGUAGE cpp SPECS ${specs} JSON XML BSON)

# The same C program, built with skywire.pc for the runtime instead of the
# CMake package.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SKYWIRE_PC REQUIRED IMPORTED_TARGET skywire)
    skywire_generate(LANGUAGE c SPECS ${specs} OUT_DIR skywire_pc OUT_VAR pc_sources JSON XML BSON)
    add_executable(consumer_pc ${EXAMPLES_DIR}/c/main.c ${pc_sources})
    target_include_directories(consumer_pc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/skywire_pc)
    target_link_libraries(consumer_pc PRIVATE PkgConfig::SKYWIRE_PC)
endif()
