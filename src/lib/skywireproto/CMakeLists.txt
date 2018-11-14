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

set(TARGETNAME skywireproto)

set(PROTO_FILES
    stencil
)

set(PROTO_CPP_DIR "${PROJECT_BINARY_DIR}/proto/cpp")
set(PROTO_PYTHON_DIR "${PROJECT_BINARY_DIR}/proto/python")

if(UNIX)
    file(MAKE_DIRECTORY ${PROTO_CPP_DIR}) 
    file(MAKE_DIRECTORY ${PROTO_PYTHON_DIR})
endif(UNIX)

foreach(PROTO ${PROTO_FILES})
    #cpp
    add_custom_command(
        OUTPUT  ${PROTO_CPP_DIR}/${PROTO}.pb.h
                ${PROTO_CPP_DIR}/${PROTO}.pb.cc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS    ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
                --cpp_out=${PROTO_CPP_DIR}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    )
    set(PROTOCPP_RESULTS ${PROTOCPP_RESULTS}
        ${PROTO_CPP_DIR}/${PROTO}.pb.cc
        ${PROTO_CPP_DIR}/${PROTO}.pb.h
    )
    #python
    add_custom_command(
        OUTPUT  ${PROTO_PYTHON_DIR}/${PROTO}.pb.py
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
        COMMAND ${PROTOC}
        ARGS    --python_out=${PROTO_PYTHON_DIR}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
    )
    set(PROTOPY_RESULTS ${PROTOPY_RESULTS}
        ${PROTO_PYTHON_DIR}/${PROTO}.pb.py
    )
endforeach(PROTO)

add_library(${TARGETNAME} STATIC ${PROTOCPP_RESULTS})

target_link_libraries(${TARGETNAME} ${PROTOBUF_LIBRARIES})
