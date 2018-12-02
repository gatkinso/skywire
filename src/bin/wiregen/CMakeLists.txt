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

set(EXAMPLE_MESSAGE_JSON ${PROJECT_SOURCE_DIR}/cfg/cpp.testspace.MyTestClass.json)
set(LANGUAGE_JSONS ${PROJECT_SOURCE_DIR}/lang/lang.cpp.json
                   ${PROJECT_SOURCE_DIR}/lang/lang.c.json)

add_executable(${TARGETNAME} ${_SOURCES})

add_dependencies(${TARGETNAME} skywire skywireproto cli)

target_link_libraries(${TARGETNAME} skywire skywireproto cli)

add_custom_command(TARGET ${TARGETNAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME} ${CMAKE_BINARY_DIR}/bin/${TARGETNAME})

#file(COPY ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME} DESTINATION ${CMAKE_BINARY_DIR}/bin)
install(TARGETS ${TARGETNAME} RUNTIME DESTINATION ${CMAKE_INSTALL_DIR}/bin)
install(FILES ${LANGUAGE_JSONS} DESTINATION ${CMAKE_INSTALL_DIR})
install(FILES ${EXAMPLE_MESSAGE_JSON} ${LANGUAGE_JSONS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
