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

set(LANGUAGE_JSONS lang.c.json
                   lang.cpp.json
                   lang.cpp11.json
                   go/lang.go.json
                   python/lang.python3.json)

add_executable(${TARGETNAME} ${_SOURCES})

add_dependencies(${TARGETNAME} skywire specification cli)

target_link_libraries(${TARGETNAME} skywire specification cli)

if(MSVC)
    set(TGT_NAME ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${TARGETNAME}.exe)
    set(DST_NAME ${CMAKE_BINARY_DIR}/bin/${TARGETNAME}.exe)
endif(MSVC)

if(UNIX)
    set(TGT_NAME ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME})
    set(DST_NAME ${CMAKE_BINARY_DIR}/bin/${TARGETNAME})
endif(UNIX)

add_custom_command(TARGET ${TARGETNAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${TGT_NAME} ${DST_NAME})

foreach(LANG_FILE ${LANGUAGE_JSONS})
add_custom_command(
        TARGET ${TARGETNAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE}
                ${CMAKE_CURRENT_BINARY_DIR}/${LANG_FILE})
set(LANG_JSON_PATHS ${LANG_JSON_PATHS} ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE})
endforeach(LANG_FILE)

install(TARGETS ${TARGETNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${LANG_JSON_PATHS} DESTINATION ${CMAKE_INSTALL_PREFIX})