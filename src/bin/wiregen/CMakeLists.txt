#
#  Copyright (C) 2017 Geoffrey Atkinson
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

include_directories(${PROJECT_SOURCE_DIR}/include)

set(_SOURCES main.cpp)

set(DEPLOY_JSONS   c/lang.c.json
                   cpp/lang.cpp.json
                   csharp/lang.csharp.json
                   go/lang.go.json
                   java/lang.java.json
                   javascript/lang.javascript.json
                   proto/lang.proto3.json
                   python/lang.python3.json
                   rust/lang.rust.json
                   swift/lang.swift.json)

add_executable(${TARGETNAME} ${_SOURCES})

add_dependencies(${TARGETNAME} skywire specification)

target_link_libraries(${TARGETNAME} skywire)

# Where an installed wiregen finds the lang files, relative to itself:
# ../share/skywire/lang, or ../../share/skywire/lang from vcpkg's tools/skywire.
file(RELATIVE_PATH WIREGEN_LANG_RELDIR
     /prefix/${SKYWIRE_BIN_INSTALL_DIR} /prefix/${CMAKE_INSTALL_DATADIR}/skywire/lang)
target_compile_definitions(${TARGETNAME} PRIVATE WIREGEN_LANG_RELDIR="${WIREGEN_LANG_RELDIR}")

add_custom_command(TARGET ${TARGETNAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGETNAME}> ${CMAKE_BINARY_DIR}/bin/${TARGETNAME}${CMAKE_EXECUTABLE_SUFFIX})

# wiregen --language looks for the lang files in its own directory first, so
# they are copied next to both built copies of wiregen.
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
    list(APPEND INSTALL_LANG_FILES ${PROJECT_SOURCE_DIR}/lang/${LANG_FILE})
endforeach(LANG_FILE)

add_custom_target(wiregen_lang_files ALL DEPENDS ${LANG_COPIES})
add_dependencies(${TARGETNAME} wiregen_lang_files)

# On Windows wiregen links protobuf and abseil as DLLs (vcpkg). Copy them next
# to the build/bin copy too, so it runs like the one the build made.
if(WIN32)
    set(WIREGEN_DLLS $<TARGET_RUNTIME_DLLS:${TARGETNAME}>)
    add_custom_command(TARGET ${TARGETNAME} POST_BUILD
                       COMMAND "$<$<BOOL:${WIREGEN_DLLS}>:${CMAKE_COMMAND};-E;copy_if_different;${WIREGEN_DLLS};${CMAKE_BINARY_DIR}/bin>"
                       COMMAND_EXPAND_LISTS)
endif()

# wiregen goes in <prefix>/bin (SKYWIRE_BIN_INSTALL_DIR), with the DLLs it needs
# on Windows unless SKYWIRE_INSTALL_DLLS is off, and the lang files in
# <prefix>/share/skywire/lang, where it looks when they aren't beside it.
if(WIN32 AND SKYWIRE_INSTALL_DLLS)
    install(TARGETS ${TARGETNAME} EXPORT skywireTargets
            RUNTIME_DEPENDENCY_SET wiregen_dlls
            RUNTIME DESTINATION ${SKYWIRE_BIN_INSTALL_DIR})
    install(RUNTIME_DEPENDENCY_SET wiregen_dlls
            PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
            POST_EXCLUDE_REGEXES ".*[Ss]ystem32/.*\\.dll"
            DIRECTORIES $<TARGET_RUNTIME_DLL_DIRS:${TARGETNAME}>
            RUNTIME DESTINATION ${SKYWIRE_BIN_INSTALL_DIR})
else()
    install(TARGETS ${TARGETNAME} EXPORT skywireTargets
            RUNTIME DESTINATION ${SKYWIRE_BIN_INSTALL_DIR})
endif()
install(FILES ${INSTALL_LANG_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/skywire/lang)
