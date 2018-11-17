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

set(TARGETNAME unit_tests)

include_directories(
                ${PROJECT_BINARY_DIR}/proto/cpp
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/lang/cpp)

set(UNIT_TESTS
                cppgenerator_unit_test.cpp
                cppresults_unit_test.cpp
                filewriter_unit_test.cpp
                generator_unit_test.cpp
                processor_unit_test.cpp
                stencil_unit_test.cpp)

set(UNIT_TEST_JSON ${PROJECT_SOURCE_DIR}/cfg/stencil_unit_test.json)

add_executable(${TARGETNAME} main.cpp ${UNIT_TESTS})

target_link_libraries(${TARGETNAME} stencil skywire GTest::GTest GTest::Main)

gtest_discover_tests(${TARGETNAME} TEST_PREFIX skywire_unit_test:)

add_dependencies(${TARGETNAME} stencil skywire)

install(TARGETS ${TARGETNAME} DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install(FILES ${UNIT_TEST_JSON} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

