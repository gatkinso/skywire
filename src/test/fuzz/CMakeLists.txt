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

#  Fuzz targets (see README.md). With -DSKYWIRE_FUZZ=ON each is a libFuzzer
#  fuzzer; otherwise it is a program that replays its corpus, and ctest runs
#  it as skywire_fuzz:<target> on every platform.

set(FUZZ_DIR ${PROJECT_BINARY_DIR}/fuzz)
set(RING_SRC_DIR ${PROJECT_SOURCE_DIR}/src/test/generated_tests/ring)
set(RING_SPEC ${RING_SRC_DIR}/RingyMcRingface.json)
set(RING_INNER_SPEC ${RING_SRC_DIR}/RingyInner.json)
set(CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus)

# The decoder targets fuzz the ring message's generated C and C++, written
# to fuzz/c and fuzz/cpp; fuzz_c_cpp_diff includes both, by those paths.
set(c_outputs ${FUZZ_DIR}/c/RingyMcRingface.h ${FUZZ_DIR}/c/RingyMcRingface.c
              ${FUZZ_DIR}/c/RingyInner.h ${FUZZ_DIR}/c/RingyInner.c)
set(cpp_outputs ${FUZZ_DIR}/cpp/RingyMcRingface.h ${FUZZ_DIR}/cpp/RingyInner.h)

foreach(lang c cpp)
    if(lang STREQUAL "c")
        set(langfile ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json)
    else()
        set(langfile ${PROJECT_SOURCE_DIR}/lang/cpp/lang.cpp.json)
    endif()
    file(MAKE_DIRECTORY ${FUZZ_DIR}/${lang})
    add_custom_command(
        OUTPUT  ${${lang}_outputs}
        DEPENDS ${RING_SPEC} ${RING_INNER_SPEC} ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen> --input_file ${RING_INNER_SPEC}
                --outdir ${FUZZ_DIR}/${lang} --language ${lang} --write_to_file
        COMMAND $<TARGET_FILE:wiregen> --input_file ${RING_SPEC}
                --outdir ${FUZZ_DIR}/${lang} --language ${lang} --write_to_file
        VERBATIM)
endforeach()

# The C target builds the generated C as C.
add_library(fuzz_ring_c STATIC ${FUZZ_DIR}/c/RingyMcRingface.c ${FUZZ_DIR}/c/RingyInner.c)
target_include_directories(fuzz_ring_c PUBLIC ${FUZZ_DIR} PRIVATE ${PROJECT_SOURCE_DIR}/lang/c)

add_custom_target(fuzz_ring_cpp DEPENDS ${cpp_outputs})

# Builds every fuzz target: cmake --build <dir> --target fuzzers.
add_custom_target(fuzzers)

# skywire_fuzz(<target> SEEDS <file or directory>... [DICT <file>]) adds the
# target and its corpus replay test. Its checked-in corpus directory,
# corpus/<target>, holds extra seeds and the inputs of fixed crashes, and is
# replayed too when it exists. For scripts/run_fuzzers.py the target is
# described in <build>/fuzz/<target>.fuzz: its program, dictionary and seeds.
function(skywire_fuzz target)
    cmake_parse_arguments(FUZZ "" "DICT" "SEEDS" ${ARGN})
    set(sources ${target}.cpp)
    if(NOT SKYWIRE_FUZZ)
        list(APPEND sources replay_main.cpp)
    endif()
    add_executable(${target} ${sources})
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FUZZ_DIR}
        ${PROJECT_SOURCE_DIR}/lang/c
        ${PROJECT_SOURCE_DIR}/lang/cpp
        ${PROTOZERO_INC_DIR})
    if(SKYWIRE_FUZZ)
        target_link_options(${target} PRIVATE -fsanitize=fuzzer)
    endif()
    add_dependencies(fuzzers ${target})

    set(corpus ${CORPUS_DIR}/${target})
    if(EXISTS ${corpus})
        list(APPEND FUZZ_SEEDS ${corpus})
    endif()
    if(SKYWIRE_FUZZ)
        list(JOIN FUZZ_SEEDS "|" seeds)
        add_test(NAME skywire_fuzz:${target}
            COMMAND ${CMAKE_COMMAND} -DFUZZER=$<TARGET_FILE:${target}> "-DSEEDS=${seeds}"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/replay_corpus.cmake)
    else()
        add_test(NAME skywire_fuzz:${target} COMMAND ${target} ${FUZZ_SEEDS})
    endif()

    set(manifest "program=$<TARGET_FILE:${target}>\nfuzzing=${SKYWIRE_FUZZ}\n")
    if(FUZZ_DICT)
        string(APPEND manifest "dict=${FUZZ_DICT}\n")
    endif()
    foreach(seed IN LISTS FUZZ_SEEDS)
        string(APPEND manifest "seed=${seed}\n")
    endforeach()
    file(GENERATE OUTPUT ${FUZZ_DIR}/${target}.fuzz CONTENT "${manifest}")
endfunction()

# The SW targets share corpus/sw: inputs a byte short, a byte long, empty,
# and all ones (NaNs, negative bitfields).
set(RING_BIN ${RING_SRC_DIR}/RingyMcRingface.bin)

skywire_fuzz(fuzz_c_sw SEEDS ${RING_BIN} ${CORPUS_DIR}/sw)
target_link_libraries(fuzz_c_sw fuzz_ring_c)

skywire_fuzz(fuzz_cpp_sw SEEDS ${RING_BIN} ${CORPUS_DIR}/sw)
add_dependencies(fuzz_cpp_sw fuzz_ring_cpp)

skywire_fuzz(fuzz_c_cpp_diff SEEDS ${RING_BIN} ${CORPUS_DIR}/sw)
target_link_libraries(fuzz_c_cpp_diff fuzz_ring_c)
add_dependencies(fuzz_c_cpp_diff fuzz_ring_cpp)

skywire_fuzz(fuzz_cpp_pz SEEDS ${PROJECT_SOURCE_DIR}/examples/Telemetry.pb)
add_dependencies(fuzz_cpp_pz fuzz_ring_cpp)

# The wiregen targets: every spec and lang file in the tree is a seed.
file(GLOB spec_seeds
    ${PROJECT_SOURCE_DIR}/src/test/generated_tests/*.json
    ${RING_SRC_DIR}/*.json
    ${PROJECT_SOURCE_DIR}/examples/specs/*.json
    ${PROJECT_SOURCE_DIR}/cfg/*.json)
file(GLOB lang_seeds ${PROJECT_SOURCE_DIR}/lang/*/lang.*.json)

foreach(target fuzz_spec fuzz_lang)
    if(target STREQUAL "fuzz_spec")
        skywire_fuzz(${target} SEEDS ${spec_seeds} DICT ${CMAKE_CURRENT_SOURCE_DIR}/json.dict)
    else()
        skywire_fuzz(${target} SEEDS ${lang_seeds} DICT ${CMAKE_CURRENT_SOURCE_DIR}/json.dict)
    endif()
    target_include_directories(${target} PRIVATE
        ${PROJECT_SOURCE_DIR}/include/skywire
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/skywireproto)
    target_compile_definitions(${target} PRIVATE
        SKYWIRE_LANG_DIR="${PROJECT_SOURCE_DIR}/lang"
        SKYWIRE_RING_SPEC="${RING_SPEC}"
        SKYWIRE_RING_INNER_SPEC="${RING_INNER_SPEC}")
    target_link_libraries(${target} skywire)
endforeach()
