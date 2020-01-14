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

set(TARGETNAME skywire_unit_tests)

include_directories(
                ${PROJECT_BINARY_DIR}/skywireproto
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/lang/cpp
                ${PROJECT_SOURCE_DIR}/lang/c)

set(WIREGEN_C_FILES
                ${PROJECT_SOURCE_DIR}/lang/c/cstream.c)

set(WIREGEN_CPP_FILES)

if(UNIX)
    set_source_files_properties(${WIREGEN_C_FILES} PROPERTIES COMPILE_FLAGS -std=c99)
endif(UNIX)

set(UNIT_TESTS
                cgenerator_unit_test.cpp
                cppgenerator_unit_test.cpp
                cstream_unit_test.cpp
                endian_unit_test.cpp
                filewriter_unit_test.cpp
                generator_unit_test.cpp
                processor_unit_test.cpp
                specification_unit_test.cpp
                stream_unit_test.cpp)

set(UNIT_TEST_JSON specification_unit_test.json)

add_executable(${TARGETNAME} main.cpp ${WIREGEN_C_FILES} ${WIREGEN_CPP_FILES} ${UNIT_TESTS})

test_big_endian(IS_BIG_ENDIAN)

if(IS_BIG_ENDIAN)
    target_compile_definitions(${TARGETNAME} PRIVATE SW_BIG_ENDIAN=1 SW_LITTLE_ENDIAN=0)
else()
    target_compile_definitions(${TARGETNAME} PRIVATE SW_BIG_ENDIAN=0 SW_LITTLE_ENDIAN=1)
endif()

if(MSVC)
    set(PLATFORM_LINK_LIBS ws2_32)
endif(MSVC)

target_link_libraries(${TARGETNAME} skywire skywireproto specification GTest::GTest GTest::Main ${PLATFORM_LINK_LIBS})

gtest_discover_tests(${TARGETNAME} TEST_PREFIX skywire_unit_test:)

add_dependencies(${TARGETNAME} wiregen)

add_custom_command(
    TARGET ${TARGETNAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/cfg/${UNIT_TEST_JSON}
        ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_JSON})


