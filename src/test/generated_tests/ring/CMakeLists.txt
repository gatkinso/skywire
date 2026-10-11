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

#  Cross-language serial test: one serialized message is passed through a relay
#  generated for every language, in a different random order each run. See
#  README.md.

set(RING_DIR ${PROJECT_BINARY_DIR}/ring)
# One spec serves every language: file names and runtime includes come from
# each lang file.
set(RING_SPEC ${CMAKE_CURRENT_SOURCE_DIR}/RingyMcRingface.json)

# The C and C++ relays build with the project; the other languages are
# generated and compiled by ring_serial.cmake when the test runs.
foreach(lang c cpp)
    if(lang STREQUAL "c")
        set(langfile ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json)
        set(outputs ${RING_DIR}/c/RingyMcRingface.h ${RING_DIR}/c/RingyMcRingface.c ${RING_DIR}/c/RingyMcRingfaceRelay.c)
        set(sources ${RING_DIR}/c/RingyMcRingface.c ${RING_DIR}/c/RingyMcRingfaceRelay.c ${PROJECT_SOURCE_DIR}/lang/c/cstream.c)
    else()
        set(langfile ${PROJECT_SOURCE_DIR}/lang/cpp/lang.cpp.json)
        set(outputs ${RING_DIR}/cpp/RingyMcRingface.h ${RING_DIR}/cpp/RingyMcRingfaceRelay.cpp)
        # stream.h swaps float/double bytes with sw_swap32/64 from cstream.c.
        set(sources ${RING_DIR}/cpp/RingyMcRingfaceRelay.cpp ${PROJECT_SOURCE_DIR}/lang/c/cstream.c)
    endif()

    file(MAKE_DIRECTORY ${RING_DIR}/${lang})
    add_custom_command(
        OUTPUT  ${outputs}
        DEPENDS ${RING_SPEC} ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${RING_SPEC}
                --outdir ${RING_DIR}/${lang}
                --language ${lang}
                --generate_relay
                --write_to_file)

    add_executable(ring_relay_${lang} ${sources})
    target_include_directories(ring_relay_${lang} PRIVATE
        ${RING_DIR}/${lang}
        ${PROJECT_SOURCE_DIR}/lang/c
        ${PROJECT_SOURCE_DIR}/lang/cpp)
endforeach()

add_executable(ring_tamper ring_tamper.cpp)

add_test(NAME skywire_ring:Serial
    COMMAND ${CMAKE_COMMAND}
            -DWIREGEN=$<TARGET_FILE:wiregen>
            -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/RingyMcRingface.bin
            -DSPEC=${RING_SPEC}
            -DLANG_DIR=${PROJECT_SOURCE_DIR}/lang
            -DWORKDIR=${RING_DIR}/run
            -DC_RELAY=$<TARGET_FILE:ring_relay_c>
            -DCPP_RELAY=$<TARGET_FILE:ring_relay_cpp>
            -DTAMPER=$<TARGET_FILE:ring_tamper>
            -DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/ring_serial.cmake)

set_tests_properties(skywire_ring:Serial PROPERTIES
    SKIP_REGULAR_EXPRESSION "RING_NOT_ENOUGH_LANGS")
