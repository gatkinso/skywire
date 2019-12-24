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

set(TARGETNAME generated_tests)

set(WG_C_FILES
    StructyMcStructface
)

set(WG_CPP_FILES
    ClassyMcClassface
)

set(WG_C_RUMTIME
               ${PROJECT_SOURCE_DIR}/lang/c/cstream.c)

set(WG_C_DIR "${PROJECT_BINARY_DIR}/generated_c_wiregen")
set(WG_CPP_DIR "${PROJECT_BINARY_DIR}/generated_cpp_wiregen")

file(MAKE_DIRECTORY ${WG_C_DIR})
file(MAKE_DIRECTORY ${WG_CPP_DIR})

include_directories(
                ${PROJECT_BINARY_DIR}/proto/cpp
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/lang/cpp
                ${PROJECT_SOURCE_DIR}/lang/c
                ${WG_C_DIR}
                ${WG_CPP_DIR})

set(UNIT_TESTS c_cpp.cpp)

# Generate C 
foreach(WG_C ${WG_C_FILES})
    add_custom_command(
        OUTPUT  ${WG_C_DIR}/${WG_C}.h
                ${WG_C_DIR}/${WG_C}.c
                ${WG_C_DIR}/${WG_C}Test.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${WG_C}.json
        COMMAND ${CMAKE_BINARY_DIR}/bin/wiregen
        ARGS    --input_file ${CMAKE_CURRENT_SOURCE_DIR}/${WG_C}.json
                --outdir ${WG_C_DIR}
                --langfile ${CMAKE_SOURCE_DIR}/lang/lang.c.json
                --generate_tests)
    set(WG_C_RESULTS ${WG_C_RESULTS}
        ${WG_C_DIR}/${WG_C}.h
        ${WG_C_DIR}/${WG_C}.c)
    set(UNIT_TESTS ${UNIT_TESTS}
        ${WG_C_DIR}/${WG_C}Test.cpp)
endforeach(WG_C)

# Generate Cpp
foreach(WG_CPP ${WG_CPP_FILES})
    add_custom_command(
        OUTPUT  ${WG_CPP_DIR}/${WG_CPP}.h ${WG_CPP_DIR}/${WG_CPP}Test.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${WG_CPP}.json
        COMMAND ${CMAKE_BINARY_DIR}/bin/wiregen
        ARGS    --input_file ${CMAKE_CURRENT_SOURCE_DIR}/${WG_CPP}.json
                --outdir ${WG_CPP_DIR}
                --langfile ${CMAKE_SOURCE_DIR}/lang/lang.cpp.json
                --generate_tests)
    set(WG_CPP_RESULTS ${WG_CPP_RESULTS}
        ${WG_CPP_DIR}/${WG_CPP}.h)
    set(UNIT_TESTS ${UNIT_TESTS}
        ${WG_CPP_DIR}/${WG_CPP}Test.cpp)    
endforeach(WG_CPP)

set(GENERATED_C_FILES
                ${WG_C_RESULTS})                

set(GENERATED_CPP_FILES
                ${WG_CPP_RESULTS})

add_executable(${TARGETNAME} 
                main.cpp
                ${GENERATED_H_FILES}
                ${GENERATED_C_FILES}
                ${GENERATED_CPP_FILES}
                ${WG_C_RUMTIME}
                ${WG_CPP_RUMTIME}
                ${UNIT_TESTS})

#add_custom_command(
#    TARGET ${TARGETNAME} PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E remove
#            ${WG_C_RESULTS}
#            ${WG_CPP_RESULTS})                

test_big_endian(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
    target_compile_definitions(${TARGETNAME} PRIVATE SW_BIG_ENDIAN=1 SW_LITTLE_ENDIAN=0)
else()
    target_compile_definitions(${TARGETNAME} PRIVATE SW_BIG_ENDIAN=0 SW_LITTLE_ENDIAN=1)
endif()

if(MSVC)
    set(PLATFORM_LINK_LIBS ws2_32)
endif(MSVC)

target_link_libraries(${TARGETNAME} stencil skywire GTest::GTest GTest::Main ${PLATFORM_LINK_LIBS})

gtest_discover_tests(${TARGETNAME} TEST_PREFIX skywire_generated_tests:)

add_dependencies(${TARGETNAME} stencil skywire wiregen)

