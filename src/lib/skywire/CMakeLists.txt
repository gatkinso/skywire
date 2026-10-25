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

set(TARGETNAME skywire)

set(TARGET_INC_DIR ${PROJECT_SOURCE_DIR}/include/${TARGETNAME})

set(_INC_DIRS ${TARGET_INC_DIR}
              ${PROJECT_BINARY_DIR}/skywireproto
              ${PROJECT_SOURCE_DIR}/include
              ${PROJECT_BINARY_DIR}/proto/cpp
              ${PROJECT_SOURCE_DIR}/lang/c)

include_directories(${TARGETNAME} ${_INC_DIRS})

set(_GENERATORS
             bitfields.cpp
             cgenerator.cpp
             ctestgenerator.cpp
             cppgenerator.cpp
             cpptestgenerator.cpp
             gogenerator.cpp
             gotestgenerator.cpp
             file_writer.cpp
             javagenerator.cpp
             javatestgenerator.cpp
             proto3generator.cpp
             proto3testgenerator.cpp
             py3generator.cpp
             py3testgenerator.cpp
             naming.cpp
             rustgenerator.cpp
             rusttestgenerator.cpp
             crelaygenerator.cpp
             cpprelaygenerator.cpp
             gorelaygenerator.cpp
             javarelaygenerator.cpp
             py3relaygenerator.cpp
             rustrelaygenerator.cpp
             swiftrelaygenerator.cpp
             swiftgenerator.cpp
             swifttestgenerator.cpp
             test_values.cpp
             ${TARGET_INC_DIR}/bitfields.h
             ${TARGET_INC_DIR}/cgenerator.h
             ${TARGET_INC_DIR}/ctestgenerator.h
             ${TARGET_INC_DIR}/cppgenerator.h
             ${TARGET_INC_DIR}/cpptestgenerator.h
             ${TARGET_INC_DIR}/gogenerator.h
             ${TARGET_INC_DIR}/gotestgenerator.h
             ${TARGET_INC_DIR}/javagenerator.h
             ${TARGET_INC_DIR}/javatestgenerator.h
             ${TARGET_INC_DIR}/naming.h
             ${TARGET_INC_DIR}/proto3generator.h
             ${TARGET_INC_DIR}/proto3testgenerator.h
             ${TARGET_INC_DIR}/py3generator.h
             ${TARGET_INC_DIR}/py3testgenerator.h
             ${TARGET_INC_DIR}/relaygenerators.h
             ${TARGET_INC_DIR}/rustgenerator.h
             ${TARGET_INC_DIR}/rusttestgenerator.h
             ${TARGET_INC_DIR}/swiftgenerator.h
             ${TARGET_INC_DIR}/swifttestgenerator.h
             ${TARGET_INC_DIR}/test_values.h
)

set(_SOURCES ${TARGETNAME}.cpp
             generator.cpp
             processor.cpp)

set(_HEADERS ${TARGET_INC_DIR}/${TARGETNAME}.h
             ${TARGET_INC_DIR}/errorcodes.h
             ${TARGET_INC_DIR}/file_writer.h
             ${TARGET_INC_DIR}/generator.h
             ${TARGET_INC_DIR}/generators.h
             ${TARGET_INC_DIR}/processor.h
             ${TARGET_INC_DIR}/result_presenter.h)

add_library(${TARGETNAME} STATIC ${_HEADERS} ${_GENERATORS} ${_SOURCES})

# skywire_version.h holds the project version, the git revision and how
# wiregen was built, for skyloupe::version() and skyloupe::build_info()
# (wiregen --version, --banner). It is regenerated on every build, but only
# rewritten when it changes.
set(VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/skywire_version.h)
if(CMAKE_OSX_ARCHITECTURES)
    list(JOIN CMAKE_OSX_ARCHITECTURES "+" BUILD_ARCH)
else()
    set(BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()
set(BUILD_OPTIONS)
if(SKYWIRE_SANITIZE)
    list(APPEND BUILD_OPTIONS "sanitizers")
endif()
if(SKYWIRE_FUZZ)
    list(APPEND BUILD_OPTIONS "fuzzing")
endif()
list(JOIN BUILD_OPTIONS ", " BUILD_OPTIONS)
add_custom_target(skywire_version
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
                             -DOUTPUT=${VERSION_HEADER}
                             -DVERSION=${PROJECT_VERSION}
                             -DSKYWIRE_REVISION=${SKYWIRE_REVISION}
                             "-DCOMPILER=${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
                             -DBUILD_TYPE=$<CONFIG>
                             "-DPLATFORM=${CMAKE_SYSTEM_NAME} ${BUILD_ARCH}"
                             "-DOPTIONS=${BUILD_OPTIONS}"
                             -P ${PROJECT_SOURCE_DIR}/cmake/skywireVersion.cmake
    BYPRODUCTS ${VERSION_HEADER}
    VERBATIM)
add_dependencies(${TARGETNAME} skywire_version)
target_include_directories(${TARGETNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

set_target_properties(${TARGETNAME} PROPERTIES FOLDER "Static Libs")

add_dependencies(${TARGETNAME} skywireproto)

target_link_libraries(${TARGETNAME} specification)
