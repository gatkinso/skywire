cmake_minimum_required(VERSION 3.5)

project(skywire)

include (CTest)

foreach(path ${CMAKE_PREFIX_PATH})
  message("Path: " ${path})
endforeach(path)

find_package(Protobuf REQUIRED)
find_package(GTest REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})

enable_testing()

if(UNIX)
    add_compile_options(-fPIC -std=c++11)
endif(UNIX)

if(MSVC)
# Set compiler options.
set(configs
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
)

foreach(config ${configs})
    if(${config} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${config} "${${config}}")
    endif()
endforeach()
endif(MSVC)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif(UNIX AND NOT APPLE)

add_subdirectory(3rd_party/qwikpirate)
add_subdirectory(src)
