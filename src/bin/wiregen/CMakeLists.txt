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

set(TARGETNAME wiregen)

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/3rd_party/qwikpirate/include)

set(_SOURCES main.cpp)

set(DEPLOY_JSONS   c/lang.c.json
                   cpp/lang.cpp.json
                   c++11/lang.cpp11.json
                   go/lang.go.json
                   java/lang.java.json
                   proto/lang.proto3.json
                   python/lang.python3.json
                   rust/lang.rust.json
                   swift/lang.swift.json)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_JSONS    c.test.json
                       cpp.test.json
                       go.test.json
                       proto3.test.json
                       python3.test.json
                       rust.test.json)
endif(CMAKE_BUILD_TYPE MATCHES "Debug")

#set(TEST_SCRIPTS   test_wiregen)

add_executable(${TARGETNAME} ${_SOURCES})

add_dependencies(${TARGETNAME} skywire specification cli)

target_link_libraries(${TARGETNAME} skywire cli)

if(MSVC)
    set(TGT_NAME ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${TARGETNAME}.exe)
    set(DST_NAME ${CMAKE_BINARY_DIR}/bin/${TARGETNAME}.exe)
    set(SCRIPT_SUFFIX ".bat")
endif(MSVC)

if(UNIX)
    set(TGT_NAME ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME})
    set(DST_NAME ${CMAKE_BINARY_DIR}/bin/${TARGETNAME})
    set(SCRIPT_SUFFIX ".sh")
endif(UNIX)

add_custom_command(TARGET ${TARGETNAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGETNAME}> ${CMAKE_BINARY_DIR}/bin/${TARGETNAME}${CMAKE_EXECUTABLE_SUFFIX})

# wiregen --language looks for the lang files in its own directory, so they are
# copied next to both built copies of wiregen and installed beside it.
get_property(MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(MULTI_CONFIG)
    set(LANG_DIRS ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG> ${CMAKE_BINARY_DIR}/bin)
else()
    set(LANG_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin)
endif()

set(LANG_COPIES)
foreach(LANG_FILE ${DEPLOY_JSONS})
    get_filename_component(LANG_NAME ${LANG_FILE} NAME)
    set(COPIES)
    set(COMMANDS)
    foreach(LANG_DIR ${LANG_DIRS})
        list(APPEND COPIES ${LANG_DIR}/${LANG_NAME})
        list(APPEND COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory ${LANG_DIR})
        list(APPEND COMMANDS COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE} ${LANG_DIR}/${LANG_NAME})
    endforeach()
    add_custom_command(
            OUTPUT  ${COPIES}
            DEPENDS ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE}
            ${COMMANDS})
    list(APPEND LANG_COPIES ${COPIES})
    set(DEPLOY_FILES ${DEPLOY_FILES} ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE})
endforeach(LANG_FILE)

add_custom_target(wiregen_lang_files ALL DEPENDS ${LANG_COPIES})
add_dependencies(${TARGETNAME} wiregen_lang_files)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    foreach(_FILE ${DEBUG_JSONS})
        add_custom_command(
                TARGET ${TARGETNAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${PROJECT_SOURCE_DIR}/cfg/${_FILE}
                        ${CMAKE_CURRENT_BINARY_DIR}/${_FILE})
        set(DEPLOY_FILES ${DEPLOY_FILES} ${PROJECT_SOURCE_DIR}/cfg/${_FILE})
    endforeach(_FILE)
endif(CMAKE_BUILD_TYPE MATCHES "Debug")

foreach(_SCRIPT ${TEST_SCRIPTS})
    set(DEPLOY_FILES ${DEPLOY_FILES} ${PROJECT_SOURCE_DIR}/scripts/${_SCRIPT}${SCRIPT_SUFFIX})
endforeach(_SCRIPT ${TEST_SCRIPTS})

install(TARGETS ${TARGETNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${DEPLOY_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX})