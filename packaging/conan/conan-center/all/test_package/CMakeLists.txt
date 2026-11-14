cmake_minimum_required(VERSION 3.15)
project(test_package LANGUAGES C CXX)

find_package(skywire REQUIRED CONFIG)

# wiregen generates the message at build time, as a project using skywire
# would: C, and C++ (which also needs protozero).
add_executable(test_package_c test_package.c)
skywire_generate(TARGET test_package_c LANGUAGE c JSON
                 SPECS ${CMAKE_CURRENT_SOURCE_DIR}/Point.json)

add_executable(test_package_cpp test_package.cpp)
target_compile_features(test_package_cpp PRIVATE cxx_std_17)
skywire_generate(TARGET test_package_cpp LANGUAGE cpp JSON
                 SPECS ${CMAKE_CURRENT_SOURCE_DIR}/Point.json)
