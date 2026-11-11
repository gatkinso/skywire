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

#  Installs the build into OUTDIR/prefix, checks what was installed, then
#  configures, builds and runs the consumer project against it. Each consumer
#  program must write examples/Telemetry.bin.
#
#  Required -D variables: BUILD_DIR, CONFIG, OUTDIR, CONSUMER_DIR, EXAMPLES_DIR,
#                         GENERATOR
#  Optional -D variables: PLATFORM, TOOLCHAIN_FILE, TRIPLET, VCPKG_INSTALLED_DIR,
#                         PROTOZERO_DIR, C_COMPILER, CXX_COMPILER

# Script mode starts with every policy at OLD; match the project's minimum.
cmake_minimum_required(VERSION 3.28)

set(prefix ${OUTDIR}/prefix)
set(build ${OUTDIR}/build)
file(REMOVE_RECURSE ${OUTDIR})

function(run_checked what)
    execute_process(${ARGN} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc})")
    endif()
endfunction()

run_checked("cmake --install"
    COMMAND ${CMAKE_COMMAND} --install ${BUILD_DIR} --prefix ${prefix} --config ${CONFIG}
    OUTPUT_QUIET)

# Everything lands under the prefix, and bin holds wiregen (and its DLLs) only.
foreach(file
        include/skywire/cstream.h
        include/skywire/sw_endian.h
        include/skywire/ctext.h
        include/skywire/cbson.h
        include/skywire/stream.h
        include/skywire/streamable.h
        include/skywire/text.h
        include/skywire/bson.h
        share/skywire/runtime/swift/SWStream.swift
        share/skywire/lang/lang.c.json
        share/skywire/lang/lang.cpp.json
        share/skywire/lang/lang.csharp.json
        share/skywire/lang/lang.go.json
        share/skywire/lang/lang.java.json
        share/skywire/lang/lang.javascript.json
        share/skywire/lang/lang.proto3.json
        share/skywire/lang/lang.python3.json
        share/skywire/lang/lang.rust.json
        share/skywire/lang/lang.swift.json
        share/skywire/schema/skywire-spec.schema.json
        share/skywire/schema/skywire-lang.schema.json)
    if(NOT EXISTS ${prefix}/${file})
        message(FATAL_ERROR "not installed: ${file}")
    endif()
endforeach()
foreach(file skywireConfig.cmake skywire.pc)
    file(GLOB_RECURSE found ${prefix}/*/${file})
    if(NOT found)
        message(FATAL_ERROR "not installed: ${file}")
    endif()
    if(file STREQUAL "skywire.pc")
        get_filename_component(pc_dir ${found} DIRECTORY)
    endif()
endforeach()
file(GLOB bin RELATIVE ${prefix}/bin ${prefix}/bin/*)
list(FILTER bin EXCLUDE REGEX "^wiregen(\\.exe)?$|\\.dll$")
if(bin)
    message(FATAL_ERROR "unexpected files in bin: ${bin}")
endif()
file(GLOB_RECURSE tests RELATIVE ${prefix} ${prefix}/*test*)
if(tests)
    message(FATAL_ERROR "test files installed: ${tests}")
endif()

set(args -S ${CONSUMER_DIR} -B ${build} -G ${GENERATOR}
         -DCMAKE_PREFIX_PATH=${prefix}
         -DCMAKE_BUILD_TYPE=${CONFIG}
         -DEXAMPLES_DIR=${EXAMPLES_DIR})
if(PLATFORM)
    list(APPEND args -A ${PLATFORM})
endif()
foreach(var TOOLCHAIN_FILE C_COMPILER CXX_COMPILER)
    if(${var})
        list(APPEND args -DCMAKE_${var}=${${var}})
    endif()
endforeach()
if(TRIPLET)
    list(APPEND args -DVCPKG_TARGET_TRIPLET=${TRIPLET})
endif()
# The consumer has no vcpkg.json, so point vcpkg at the packages the build
# already has: its manifest's vcpkg_installed, or classic mode's installed/.
if(VCPKG_INSTALLED_DIR)
    list(APPEND args -DVCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR} -DVCPKG_MANIFEST_MODE=OFF)
endif()
if(PROTOZERO_DIR)
    list(APPEND args -DPROTOZERO_DIR=${PROTOZERO_DIR})
endif()

# skywire.pc's paths are relative to the .pc file.
set(ENV{PKG_CONFIG_PATH} ${pc_dir})
run_checked("consumer configure" COMMAND ${CMAKE_COMMAND} ${args})
run_checked("consumer build" COMMAND ${CMAKE_COMMAND} --build ${build} --config ${CONFIG})

# consumer_pc is built when the consumer's FindPkgConfig found pkg-config; ask
# its cache rather than searching here, which can find one it didn't.
load_cache(${build} READ_WITH_PREFIX consumer_ PKG_CONFIG_EXECUTABLE)
set(programs c cpp)
if(consumer_PKG_CONFIG_EXECUTABLE)
    list(APPEND programs pc)
endif()
foreach(lang ${programs})
    file(GLOB_RECURSE exe ${build}/consumer_${lang} ${build}/consumer_${lang}.exe)
    if(NOT exe)
        message(FATAL_ERROR "consumer_${lang} was not built")
    endif()
    list(GET exe 0 exe)
    run_checked("consumer_${lang}" COMMAND ${exe} ${OUTDIR}/${lang}.bin)
    run_checked("consumer_${lang} output matches Telemetry.bin"
        COMMAND ${CMAKE_COMMAND} -E compare_files ${OUTDIR}/${lang}.bin ${EXAMPLES_DIR}/Telemetry.bin)
endforeach()

# Without protozero: the c component is found, the cpp component is not, and a
# find_package that requires cpp fails and says why. The include search is
# confined to a directory that has no protozero. The project enables C: with
# no language, CMake does not search lib64, where Fedora, RHEL and Rocky put
# the package.
set(no_pz ${OUTDIR}/no_protozero)
file(MAKE_DIRECTORY ${no_pz}/empty)
file(WRITE ${no_pz}/CMakeLists.txt [=[
cmake_minimum_required(VERSION 3.28)
project(skywire_no_protozero C)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/empty)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
find_package(skywire CONFIG REQUIRED COMPONENTS c)
if(TARGET skywire::cpp_runtime)
    message(FATAL_ERROR "skywire::cpp_runtime defined without protozero")
endif()
find_package(skywire CONFIG REQUIRED COMPONENTS c cpp)
]=])
execute_process(
    COMMAND ${CMAKE_COMMAND} -S ${no_pz} -B ${no_pz}/build -DCMAKE_PREFIX_PATH=${prefix}
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
if(rc EQUAL 0)
    message(FATAL_ERROR "find_package(skywire COMPONENTS cpp) succeeded without protozero")
endif()
if(NOT out MATCHES "needs protozero" OR out MATCHES "cpp_runtime defined without")
    message(FATAL_ERROR "missing protozero not reported as expected:\n${out}")
endif()
