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
# Runs wiregen for one language on SPECS with --generate_tests,
# --generate_relay and any EXTRA_FLAGS, and compares what it writes, byte for
# byte, with the files in EXPECTED. With UPDATE=ON, replaces EXPECTED with
# the output.
#
#   cmake -DWIREGEN=... -DLANGUAGE=c -DSPECS="a.json|b.json" -DEXPECTED=dir
#         -DOUT_DIR=dir [-DEXTRA_FLAGS="--x|--y"] [-DUPDATE=ON] -P golden.cmake

foreach(VAR WIREGEN LANGUAGE SPECS EXPECTED OUT_DIR)
    if(NOT DEFINED ${VAR})
        message(FATAL_ERROR "golden.cmake: ${VAR} is not set")
    endif()
endforeach()

string(REPLACE "|" ";" SPECS "${SPECS}")
string(REPLACE "|" ";" EXTRA_FLAGS "${EXTRA_FLAGS}")

file(REMOVE_RECURSE ${OUT_DIR})
file(MAKE_DIRECTORY ${OUT_DIR})
foreach(SPEC IN LISTS SPECS)
    execute_process(
        COMMAND ${WIREGEN} --language ${LANGUAGE} --generate_tests --generate_relay ${EXTRA_FLAGS}
                --silent --write_to_file --outdir ${OUT_DIR} --input_file ${SPEC}
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE OUTPUT)
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "wiregen failed for ${SPEC}:\n${OUTPUT}")
    endif()
endforeach()

file(GLOB GOT RELATIVE ${OUT_DIR} ${OUT_DIR}/*)
list(SORT GOT)

if(UPDATE)
    file(REMOVE_RECURSE ${EXPECTED})
    file(MAKE_DIRECTORY ${EXPECTED})
    foreach(NAME IN LISTS GOT)
        file(COPY ${OUT_DIR}/${NAME} DESTINATION ${EXPECTED})
    endforeach()
    list(LENGTH GOT COUNT)
    message(STATUS "${LANGUAGE}: wrote ${COUNT} files to ${EXPECTED}")
    return()
endif()

file(GLOB WANT RELATIVE ${EXPECTED} ${EXPECTED}/*)
list(SORT WANT)

set(PROBLEMS "")
foreach(NAME IN LISTS WANT)
    if(NOT EXISTS ${OUT_DIR}/${NAME})
        string(APPEND PROBLEMS "  not written: ${NAME}\n")
    endif()
endforeach()
foreach(NAME IN LISTS GOT)
    if(NOT EXISTS ${EXPECTED}/${NAME})
        string(APPEND PROBLEMS "  not expected: ${NAME}\n")
        continue()
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E compare_files ${OUT_DIR}/${NAME} ${EXPECTED}/${NAME}
        RESULT_VARIABLE DIFFERENT)
    if(DIFFERENT)
        string(APPEND PROBLEMS "  differs: ${NAME}\n")
    endif()
endforeach()

if(PROBLEMS)
    message(FATAL_ERROR
        "wiregen's ${LANGUAGE} output is not what ${EXPECTED} holds:\n"
        "${PROBLEMS}"
        "Compare ${OUT_DIR} with ${EXPECTED}. If the change is meant, build the\n"
        "skywire_update_golden target and commit the new files.")
endif()
