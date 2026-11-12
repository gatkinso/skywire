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
# skywire_text_diff:<language>: each language's generated JSON, XML and BSON
# code against the C++, on the fuzz corpora and seeded mutations of them
# (text_diff.cmake). The C code has its own check, the fuzz_c_cpp_text
# fuzz target.

add_executable(text_diff_oracle oracle.cpp)
target_include_directories(text_diff_oracle PRIVATE
    ${PROJECT_BINARY_DIR}/fuzz
    ${PROJECT_SOURCE_DIR}/lang/cpp
    ${PROJECT_SOURCE_DIR}/lang/c
    ${PROTOZERO_INC_DIR})
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    target_compile_definitions(text_diff_oracle PRIVATE SW_BIG_ENDIAN=1 SW_LITTLE_ENDIAN=0)
else()
    target_compile_definitions(text_diff_oracle PRIVATE SW_BIG_ENDIAN=0 SW_LITTLE_ENDIAN=1)
endif()
add_dependencies(text_diff_oracle fuzz_ring_cpp_text)

find_program(SKYWIRE_PYTHON3 NAMES python3 python)
find_program(SKYWIRE_GO go)
if(NOT SKYWIRE_GO)
    set(SKYWIRE_GO "")
endif()
set(corpus ${PROJECT_SOURCE_DIR}/src/test/fuzz/corpus)
set(corpora "${corpus}/text|${corpus}/fuzz_cpp_json|${corpus}/fuzz_cpp_xml|${corpus}/fuzz_cpp_bson")

foreach(language python3 go)
    add_test(NAME skywire_text_diff:${language}
        COMMAND ${CMAKE_COMMAND}
                -DLANGUAGE=${language}
                -DWIREGEN=$<TARGET_FILE:wiregen>
                -DORACLE=$<TARGET_FILE:text_diff_oracle>
                -DPYTHON=${SKYWIRE_PYTHON3}
                -DGO=${SKYWIRE_GO}
                -DCOUNT=30000
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                -DRING_DIR=${PROJECT_SOURCE_DIR}/src/test/generated_tests/ring
                -DCORPORA=${corpora}
                -DOUTDIR=${CMAKE_CURRENT_BINARY_DIR}/${language}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/text_diff.cmake)
    set_tests_properties(skywire_text_diff:${language} PROPERTIES SKIP_REGULAR_EXPRESSION "TEXT_DIFF_SKIPPED")
endforeach()
