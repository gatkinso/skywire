#
#  Copyright (C) 2026 Skyloupe, LLC
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

#  Benchmarks of the generated code (SKYWIRE_BENCH=ON). Not part of ctest.
#  See README.md.

get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT multi_config AND NOT CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo)$")
    message(WARNING "SKYWIRE_BENCH: CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'; "
                    "use Release for meaningful numbers")
endif()
if(SKYWIRE_SANITIZE)
    message(WARNING "SKYWIRE_BENCH: sanitizers are on; the numbers will be meaningless")
endif()

# The ring message, the same one the cross-language serial test uses.
set(RING_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/test/generated_tests/ring)
set(BENCH_DIR ${PROJECT_BINARY_DIR}/bench)

foreach(lang c cpp)
    if(lang STREQUAL "c")
        set(langfile ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json)
        set(outputs ${BENCH_DIR}/c/RingyMcRingface.h ${BENCH_DIR}/c/RingyMcRingface.c)
        set(inner_outputs ${BENCH_DIR}/c/RingyInner.h ${BENCH_DIR}/c/RingyInner.c)
        set(sources bench_c.c ${BENCH_DIR}/c/RingyMcRingface.c ${BENCH_DIR}/c/RingyInner.c
                    ${PROJECT_SOURCE_DIR}/lang/c/cstream.c)
    else()
        set(langfile ${PROJECT_SOURCE_DIR}/lang/cpp/lang.cpp.json)
        set(outputs ${BENCH_DIR}/cpp/RingyMcRingface.h)
        set(inner_outputs ${BENCH_DIR}/cpp/RingyInner.h)
        set(sources bench_cpp.cpp ${BENCH_DIR}/cpp/RingyMcRingface.h ${BENCH_DIR}/cpp/RingyInner.h)
    endif()

    file(MAKE_DIRECTORY ${BENCH_DIR}/${lang})
    add_custom_command(
        OUTPUT  ${inner_outputs}
        DEPENDS ${RING_SOURCE_DIR}/RingyInner.json ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${RING_SOURCE_DIR}/RingyInner.json
                --outdir ${BENCH_DIR}/${lang}
                --language ${lang}
                --write_to_file)

    add_custom_command(
        OUTPUT  ${outputs}
        DEPENDS ${RING_SOURCE_DIR}/RingyMcRingface.json ${RING_SOURCE_DIR}/RingyInner.json ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${RING_SOURCE_DIR}/RingyMcRingface.json
                --outdir ${BENCH_DIR}/${lang}
                --language ${lang}
                --write_to_file)

    add_executable(bench_${lang} ${sources})
    target_include_directories(bench_${lang} PRIVATE
        ${BENCH_DIR}/${lang}
        ${PROJECT_SOURCE_DIR}/lang/c
        ${PROJECT_SOURCE_DIR}/lang/cpp
        ${PROTOZERO_INC_DIR})
endforeach()

# timespec_get is C11.
set_target_properties(bench_c PROPERTIES C_STANDARD 11)

# `cmake --build . --target bench` builds the C and C++ benchmarks and runs
# every language through run.sh.
if(WIN32)
    # Use Git's bash: the bash.exe in System32 runs WSL, which cannot run the
    # Windows toolchains.
    find_package(Git QUIET)
    if(GIT_FOUND)
        get_filename_component(git_root ${GIT_EXECUTABLE} DIRECTORY)
        get_filename_component(git_root ${git_root} DIRECTORY)
        find_program(BASH_EXECUTABLE bash HINTS ${git_root}/bin NO_DEFAULT_PATH)
    endif()
else()
    find_program(BASH_EXECUTABLE bash)
endif()
if(BASH_EXECUTABLE)
    add_custom_target(bench
        COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run.sh
                --wiregen $<TARGET_FILE:wiregen>
                --c $<TARGET_FILE:bench_c>
                --cpp $<TARGET_FILE:bench_cpp>
                --workdir ${BENCH_DIR}/run
        DEPENDS wiregen bench_c bench_cpp
        USES_TERMINAL)
endif()
