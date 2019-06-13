set(TARGETNAME generator)
set(${TARGETNAME}_INC_DIR ${PROJECT_SOURCE_DIR}/include/${TARGETNAME})

set(Python_ADDITIONAL_VERSIONS 3.6)
find_package(Python3 COMPONENTS Development)
 
include_directories(
                   ${${TARGETNAME}_INC_DIR}
                   ${PROJECT_SOURCE_DIR}/c++/include/apm
                   ${PROJECT_BINARY_DIR}/proto/cpp
                   ${Python3_INCLUDE_DIRS}
)

set(LINK_LIBS ${Python3_LIBRARIES})
set(${TARGETNAME}_SOURCES ${TARGETNAME}.cpp)
set(BUILD_SHARED_LIBS TRUE)

add_compile_options(-fPIC)

add_library(${TARGETNAME} SHARED ${${TARGETNAME}_SOURCES})

target_link_libraries(${TARGETNAME} ${LINK_LIBS})

add_custom_command(TARGET ${TARGETNAME} POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:${TARGETNAME}>"
     "${CMAKE_BINARY_DIR}/staging/$<TARGET_FILE_NAME:${TARGETNAME}>"
  COMMENT "Copying to output directory"
)

install(TARGETS ${TARGETNAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)