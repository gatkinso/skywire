# skywire's CMake package without its own config files: the wiregen target,
# the variables the package documents, and skywire_generate().

if(NOT TARGET skywire::wiregen)
    add_executable(skywire::wiregen IMPORTED)
    set_property(TARGET skywire::wiregen PROPERTY IMPORTED_LOCATION
        "${CMAKE_CURRENT_LIST_DIR}/../../../bin/wiregen${CMAKE_EXECUTABLE_SUFFIX}")
endif()

get_filename_component(_skywire_root "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
set(SKYWIRE_INCLUDE_DIR "${_skywire_root}/include/skywire")
set(SKYWIRE_RUNTIME_DIR "${_skywire_root}/share/skywire/runtime")
set(SKYWIRE_LANG_DIR "${_skywire_root}/share/skywire/lang")
unset(_skywire_root)

include("${CMAKE_CURRENT_LIST_DIR}/skywireGenerate.cmake")
