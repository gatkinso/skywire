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

#  Example programs (SKYWIRE_EXAMPLES=ON), one per language. Each one is a
#  ctest, skywire_example:<Lang>, that checks the program writes Telemetry.bin.
#  See README.md.

set(SPECS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/specs)
set(EXAMPLES_OUT ${PROJECT_BINARY_DIR}/examples)

# The C and C++ examples build with the project; run_example.cmake builds the
# others when the test runs.
foreach(lang c cpp)
    set(dir ${EXAMPLES_OUT}/${lang})
    if(lang STREQUAL "c")
        set(langfile ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json)
        set(position_outputs ${dir}/Position.h ${dir}/Position.c)
        set(telemetry_outputs ${dir}/Telemetry.h ${dir}/Telemetry.c)
        set(sources c/main.c ${dir}/Position.c ${dir}/Telemetry.c)
    else()
        set(langfile ${PROJECT_SOURCE_DIR}/lang/cpp/lang.cpp.json)
        set(position_outputs ${dir}/Position.h)
        set(telemetry_outputs ${dir}/Telemetry.h)
        set(sources cpp/main.cpp ${dir}/Position.h ${dir}/Telemetry.h)
    endif()

    file(MAKE_DIRECTORY ${dir})
    add_custom_command(
        OUTPUT  ${position_outputs}
        DEPENDS ${SPECS_DIR}/Position.json ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${SPECS_DIR}/Position.json
                --outdir ${dir}
                --language ${lang}
                --write_to_file)
    add_custom_command(
        OUTPUT  ${telemetry_outputs}
        DEPENDS ${SPECS_DIR}/Telemetry.json ${langfile} wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${SPECS_DIR}/Telemetry.json
                --outdir ${dir}
                --language ${lang}
                --write_to_file)

    # Each language's Position.h is in its own directory, so neither sees the
    # other's.
    add_executable(example_${lang} ${sources})
    target_include_directories(example_${lang} PRIVATE
        ${dir}
        ${PROJECT_SOURCE_DIR}/lang/c
        ${PROJECT_SOURCE_DIR}/lang/cpp
        ${PROTOZERO_INC_DIR})
endforeach()

# The protobuf example: the C++ messages above in a PZ-mode stream, talking to
# protoc's C++ code for the .proto files wiregen writes. The .proto package is
# telemetry.pb, so protoc's classes (telemetry::pb) don't clash with skywire's
# (telemetry).
set(pb_dir ${EXAMPLES_OUT}/protobuf)
file(MAKE_DIRECTORY ${pb_dir})
foreach(message Position Telemetry)
    add_custom_command(
        OUTPUT  ${pb_dir}/${message}.proto
        DEPENDS ${SPECS_DIR}/${message}.json ${PROJECT_SOURCE_DIR}/lang/proto/lang.proto3.json wiregen
        COMMAND $<TARGET_FILE:wiregen>
        ARGS    --input_file ${SPECS_DIR}/${message}.json
                --outdir ${pb_dir}
                --language proto3
                --namespace telemetry::pb
                --write_to_file)
endforeach()
add_custom_command(
    OUTPUT  ${pb_dir}/Position.pb.h ${pb_dir}/Position.pb.cc
            ${pb_dir}/Telemetry.pb.h ${pb_dir}/Telemetry.pb.cc
    DEPENDS ${pb_dir}/Position.proto ${pb_dir}/Telemetry.proto
    COMMAND protobuf::protoc
    ARGS    --proto_path=${pb_dir} --cpp_out=${pb_dir}
            ${pb_dir}/Position.proto ${pb_dir}/Telemetry.proto)

add_executable(example_protobuf protobuf/main.cpp
    ${pb_dir}/Position.pb.cc ${pb_dir}/Telemetry.pb.cc
    ${EXAMPLES_OUT}/cpp/Position.h ${EXAMPLES_OUT}/cpp/Telemetry.h)
target_include_directories(example_protobuf PRIVATE
    ${EXAMPLES_OUT}/cpp
    ${pb_dir}
    ${PROJECT_SOURCE_DIR}/lang/c
    ${PROJECT_SOURCE_DIR}/lang/cpp
    ${PROTOZERO_INC_DIR})
if(MSVC)
    target_compile_options(example_protobuf PRIVATE "/wd4800")
endif()
target_link_libraries(example_protobuf protobuf::libprotobuf)

add_test(NAME skywire_example:Protobuf
    COMMAND ${CMAKE_COMMAND}
            -DLANG=protobuf
            -DEXAMPLES_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/Telemetry.pb
            -DOUTDIR=${EXAMPLES_OUT}/run/protobuf
            -DEXE=$<TARGET_FILE:example_protobuf>
            -DPROTOC=$<TARGET_FILE:protobuf::protoc>
            -DPROTO_DIR=${pb_dir}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_example.cmake)

foreach(lang c cpp swift go python java rust)
    if(lang STREQUAL "c")
        set(name C)
    elseif(lang STREQUAL "cpp")
        set(name C++)
    else()
        string(SUBSTRING ${lang} 0 1 first)
        string(TOUPPER ${first} first)
        string(SUBSTRING ${lang} 1 -1 rest)
        set(name ${first}${rest})
    endif()

    if(lang STREQUAL "c" OR lang STREQUAL "cpp")
        set(exe -DEXE=$<TARGET_FILE:example_${lang}>)
    else()
        set(exe "")
    endif()

    add_test(NAME skywire_example:${name}
        COMMAND ${CMAKE_COMMAND}
                -DLANG=${lang}
                -DEXAMPLES_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/Telemetry.bin
                -DOUTDIR=${EXAMPLES_OUT}/run/${lang}
                -DWIREGEN=$<TARGET_FILE:wiregen>
                -DLANG_DIR=${PROJECT_SOURCE_DIR}/lang
                -DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                ${exe}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/run_example.cmake)
    set_tests_properties(skywire_example:${name} PROPERTIES
        SKIP_REGULAR_EXPRESSION "SWIFT_TOOLCHAIN_NOT_FOUND|GO_TOOLCHAIN_NOT_FOUND|PYTHON3_NOT_FOUND|JAVA_TOOLCHAIN_NOT_FOUND|RUST_TOOLCHAIN_NOT_FOUND")
endforeach()
