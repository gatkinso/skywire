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

#  Builds the Swift example as a separate Swift package that gets SWStream
#  from skywire's Package.swift (the Skywire library) instead of a copy, then
#  runs it. Skipped when swift is not on PATH.
#
#  Required -D variables: WIREGEN, SOURCE_DIR, OUTDIR

# Script mode starts with every policy at OLD; match the project's minimum.
cmake_minimum_required(VERSION 3.28)

find_program(SWIFT_EXECUTABLE swift)
if(NOT SWIFT_EXECUTABLE)
    message("SWIFT_TOOLCHAIN_NOT_FOUND: swift not on PATH; skipping the SwiftPM test")
    return()
endif()

function(run_checked what)
    execute_process(${ARGN} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc})")
    endif()
endfunction()

set(package ${OUTDIR}/consumer)
set(sources ${package}/Sources/consumer)
file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${sources})

get_filename_component(PACKAGE_ID ${SOURCE_DIR} NAME)
string(TOLOWER ${PACKAGE_ID} PACKAGE_ID)
configure_file(${CMAKE_CURRENT_LIST_DIR}/swiftpm/Package.swift.in ${package}/Package.swift @ONLY)
file(COPY ${SOURCE_DIR}/examples/swift/main.swift DESTINATION ${sources})
foreach(spec Position Telemetry)
    run_checked("wiregen ${spec}"
        COMMAND ${WIREGEN} --language swift --input_file ${SOURCE_DIR}/examples/specs/${spec}.json
                --write_to_file --silent --outdir ${sources})
endforeach()

# Its own scratch path keeps SwiftPM's caches out of the source tree.
run_checked("swift build"
    COMMAND ${SWIFT_EXECUTABLE} build --package-path ${package} --scratch-path ${package}/.build
            --product consumer)
execute_process(COMMAND ${SWIFT_EXECUTABLE} build --package-path ${package} --scratch-path ${package}/.build
                        --product consumer --show-bin-path
                OUTPUT_VARIABLE bin OUTPUT_STRIP_TRAILING_WHITESPACE)
run_checked("consumer" COMMAND ${bin}/consumer ${OUTDIR}/Telemetry.bin)
if(NOT EXISTS ${OUTDIR}/Telemetry.bin)
    message(FATAL_ERROR "consumer wrote no ${OUTDIR}/Telemetry.bin")
endif()
