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
    specification
)

set(PROTO_DIR "${PROJECT_BINARY_DIR}/${TARGETNAME}")

if(UNIX)
    file(MAKE_DIRECTORY ${PROTO_DIR}) 
endif(UNIX)

foreach(PROTO ${PROTO_FILES})
    #cpp Protobuf
    add_custom_command(
        OUTPUT  ${PROTO_DIR}/${PROTO}.pb.h
                ${PROTO_DIR}/${PROTO}.pb.cc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
        COMMAND protobuf::protoc
        ARGS    ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO}.proto
                --cpp_out=${PROTO_DIR}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
    )
    set(PROTOCPP_RESULTS ${PROTOCPP_RESULTS}
        ${PROTO_DIR}/${PROTO}.pb.cc
        ${PROTO_DIR}/${PROTO}.pb.h
    )
endforeach(PROTO)

add_library(${TARGETNAME} STATIC ${PROTOCPP_RESULTS})

if(MSVC)
    target_compile_options(${TARGETNAME} PRIVATE "/wd4800")
endif(MSVC)

target_link_libraries(${TARGETNAME} protobuf::libprotobuf)
