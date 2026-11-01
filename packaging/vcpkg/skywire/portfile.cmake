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

#  The skywire overlay port. It builds the source tree it is in; an upstream
#  port would fetch the release with vcpkg_from_github, and qwikpirate (a
#  submodule) with a second one. test_port.py builds from a copy of this file
#  with SOURCE_PATH set.

get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

# wiregen is a tool and the runtimes are headers: no debug build.
set(VCPKG_BUILD_TYPE release)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
        -DSKYWIRE_EXAMPLES=OFF
        -DSKYWIRE_BIN_INSTALL_DIR=tools/${PORT}
        -DSKYWIRE_INSTALL_DLLS=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/skywire)
vcpkg_fixup_pkgconfig()
vcpkg_copy_tool_dependencies("${CURRENT_PACKAGES_DIR}/tools/${PORT}")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/doc")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
