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

set(TARGETNAME skywire)

set(TARGET_INC_DIR ${PROJECT_SOURCE_DIR}/include/${TARGETNAME})

set(_INC_DIRS ${TARGET_INC_DIR}
              ${PROJECT_BINARY_DIR}/skywireproto
              ${PROJECT_SOURCE_DIR}/include
              ${PROJECT_BINARY_DIR}/proto/cpp
              ${PROJECT_SOURCE_DIR}/lang/c)

include_directories(${TARGETNAME} ${_INC_DIRS})

set(_GENERATORS
             adagenerator.cpp
             cgenerator.cpp
             ctestgenerator.cpp
             cppgenerator.cpp
             cpptestgenerator.cpp
             gogenerator.cpp
             file_writer.cpp
             javagenerator.cpp
             py3generator.cpp
             ${TARGET_INC_DIR}/adagenerator.h
             ${TARGET_INC_DIR}/cgenerator.h
             ${TARGET_INC_DIR}/ctestgenerator.h
             ${TARGET_INC_DIR}/cppgenerator.h
             ${TARGET_INC_DIR}/cpptestgenerator.h
             ${TARGET_INC_DIR}/gogenerator.h
             ${TARGET_INC_DIR}/javagenerator.h
             ${TARGET_INC_DIR}/py3generator.h)

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

set_target_properties(${TARGETNAME} PROPERTIES FOLDER "Static Libs")

add_dependencies(${TARGETNAME} skywireproto)

target_link_libraries(${TARGETNAME} skywireproto stencil)
