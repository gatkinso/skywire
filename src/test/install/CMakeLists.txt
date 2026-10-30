#
#  Copyright (C) 2026 Geoffrey Atkinson
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

#  skywire_install:Consumer installs the build into a scratch prefix and builds
#  consumer/, a separate project that uses it through find_package(skywire).

add_test(NAME skywire_install:Consumer
    COMMAND ${CMAKE_COMMAND}
            -DBUILD_DIR=${PROJECT_BINARY_DIR}
            -DCONFIG=$<CONFIG>
            -DOUTDIR=${CMAKE_CURRENT_BINARY_DIR}/run
            -DCONSUMER_DIR=${CMAKE_CURRENT_SOURCE_DIR}/consumer
            -DEXAMPLES_DIR=${PROJECT_SOURCE_DIR}/examples
            -DGENERATOR=${CMAKE_GENERATOR}
            -DPLATFORM=${CMAKE_GENERATOR_PLATFORM}
            -DTOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DTRIPLET=${VCPKG_TARGET_TRIPLET}
            -DVCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}
            -DPROTOZERO_DIR=${PROTOZERO_DIR}
            -DC_COMPILER=${CMAKE_C_COMPILER}
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_install_test.cmake)

#  skywire_install:SwiftPM builds the Swift example as a separate Swift package
#  that depends on the Package.swift at the top of the source tree.

add_test(NAME skywire_install:SwiftPM
    COMMAND ${CMAKE_COMMAND}
            -DWIREGEN=$<TARGET_FILE:wiregen>
            -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
            -DOUTDIR=${CMAKE_CURRENT_BINARY_DIR}/swiftpm
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_swiftpm_test.cmake)
set_tests_properties(skywire_install:SwiftPM PROPERTIES
    SKIP_REGULAR_EXPRESSION "SWIFT_TOOLCHAIN_NOT_FOUND")
