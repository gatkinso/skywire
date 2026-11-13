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
#  JSON, XML and BSON example test driver, run by CTest via `cmake -P` (see
#  CMakeLists.txt). Runs one of the examples/json, examples/xml or
#  examples/bson programs on that directory's input, and checks it succeeds
#  (it checks its own refusals) and writes exactly the directory's expected
#  canonical output, which the C++, Python, Go and Swift programs must all
#  write. A missing toolchain prints the skip marker the test's
#  SKIP_REGULAR_EXPRESSION matches.
#
#  Required -D variables: FORMAT (json, xml, bson), LANG (cpp, python, go,
#                         swift), EXAMPLES_DIR, OUTDIR
#  For cpp:               EXE
#  For the others:        WIREGEN
#  For swift:             LANG_DIR
#  Optional -D variables: EXE_SUFFIX
#

cmake_minimum_required(VERSION 3.28)

set(src ${EXAMPLES_DIR}/${FORMAT})
file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})

# Generates Position and Telemetry, with the three forms, into OUTDIR; extra
# arguments go to wiregen.
function(generate lang)
    foreach(spec Position Telemetry)
        execute_process(
            COMMAND ${WIREGEN} --input_file ${EXAMPLES_DIR}/specs/${spec}.json --outdir ${OUTDIR}
                    --language ${lang} --generate_json --generate_xml --generate_bson --write_to_file ${ARGN}
            RESULT_VARIABLE rc)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
        endif()
    endforeach()
endfunction()

function(run_checked what)
    execute_process(${ARGN} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc})")
    endif()
endfunction()

if(LANG STREQUAL "cpp")
    set(run ${EXE})
elseif(LANG STREQUAL "python")
    # Windows may have a python3 that only opens the Microsoft Store.
    find_program(PYTHON3_EXECUTABLE NAMES python3 python)
    if(PYTHON3_EXECUTABLE)
        execute_process(COMMAND ${PYTHON3_EXECUTABLE} -c "import sys; print(sys.version_info[0])"
            OUTPUT_VARIABLE py_major OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    if(NOT PYTHON3_EXECUTABLE OR NOT py_major STREQUAL "3")
        message("PYTHON3_NOT_FOUND: no Python 3 interpreter on PATH; skipping the Python example")
        return()
    endif()
    generate(python)
    # Next to the generated modules, so it imports them.
    file(COPY ${src}/main.py DESTINATION ${OUTDIR})
    set(run ${PYTHON3_EXECUTABLE} ${OUTDIR}/main.py)
elseif(LANG STREQUAL "go")
    find_program(GO_EXECUTABLE go)
    if(GO_EXECUTABLE)
        # Never download a newer toolchain to satisfy go.mod.
        set(ENV{GOTOOLCHAIN} local)
        execute_process(COMMAND ${GO_EXECUTABLE} env GOVERSION
            OUTPUT_VARIABLE go_version OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX REPLACE "^go([0-9]+\\.[0-9]+).*" "\\1" go_version_num "${go_version}")
    endif()
    if(NOT GO_EXECUTABLE OR go_version_num VERSION_LESS 1.21)
        message("GO_TOOLCHAIN_NOT_FOUND: no go 1.21 or newer on PATH; skipping the Go example")
        return()
    endif()
    # The example is a command, so the messages are generated into package main.
    generate(go --namespace main)
    file(COPY ${src}/main.go DESTINATION ${OUTDIR})
    file(WRITE ${OUTDIR}/go.mod "module wiregen/example\n\ngo 1.21\n")
    get_filename_component(go_bindir ${GO_EXECUTABLE} DIRECTORY)
    find_program(GOFMT_EXECUTABLE gofmt HINTS ${go_bindir})
    if(GOFMT_EXECUTABLE)
        execute_process(COMMAND ${GOFMT_EXECUTABLE} -l .
            WORKING_DIRECTORY ${OUTDIR}
            OUTPUT_VARIABLE unformatted OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(unformatted)
            message(FATAL_ERROR "not gofmt-clean: ${unformatted}")
        endif()
    endif()
    run_checked("go vet" COMMAND ${GO_EXECUTABLE} vet . WORKING_DIRECTORY ${OUTDIR})
    set(exe ${OUTDIR}/example_${FORMAT}_go${EXE_SUFFIX})
    run_checked("go build" COMMAND ${GO_EXECUTABLE} build -o ${exe} . WORKING_DIRECTORY ${OUTDIR})
    set(run ${exe})
elseif(LANG STREQUAL "swift")
    find_program(SWIFTC_EXECUTABLE swiftc)
    if(NOT SWIFTC_EXECUTABLE)
        message("SWIFT_TOOLCHAIN_NOT_FOUND: swiftc not on PATH; skipping the Swift example")
        return()
    endif()
    generate(swift)
    set(exe ${OUTDIR}/example_${FORMAT}_swift${EXE_SUFFIX})
    run_checked("swiftc" COMMAND ${SWIFTC_EXECUTABLE} -o ${exe} ${LANG_DIR}/swift/SWStream.swift
        ${LANG_DIR}/swift/SkywireText.swift ${OUTDIR}/Position.swift ${OUTDIR}/Telemetry.swift ${src}/main.swift)
    set(run ${exe})
else()
    message(FATAL_ERROR "unknown LANG '${LANG}'")
endif()

set(written ${OUTDIR}/out.${FORMAT})
execute_process(COMMAND ${run} ${src}/telemetry.${FORMAT} ${written}
    WORKING_DIRECTORY ${OUTDIR} RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "the ${LANG} ${FORMAT} example failed (${rc})")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${written} ${src}/expected.${FORMAT}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    file(READ ${written} got HEX)
    file(READ ${src}/expected.${FORMAT} want HEX)
    message(FATAL_ERROR "the ${LANG} ${FORMAT} example wrote a different canonical form:\n"
                        "  got  ${got}\n  want ${want}")
endif()
