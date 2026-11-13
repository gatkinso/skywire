#
#   Copyright (C) 2026 Geoffrey Atkinson
#
#   Licensed under the Apache License, Version 2.0 (the "License").
#   You may not use this file except in compliance with the License.
#   A copy of the License is located at
#
#   https://www.apache.org/licenses/LICENSE-2.0.html
#
#   or in the "license" file accompanying this file. This file is distributed
#   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#   express or implied. See the License for the specific language governing
#   permissions and limitations under the License.
#
"""Conan 2 recipe for skywire, built from the source tree it is in.

    conan create packaging/conan --build=missing

The package is wiregen plus the header-only C and C++ runtimes, with skywire's
own CMake package (find_package(skywire), skywire_generate()) rather than one
CMakeDeps makes, and skywire.pc. protobuf is linked into wiregen statically
and is not visible to consumers; protozero is, since generated C++ includes it.
"""

import os
import re

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load, rmdir

# What the build needs from the source tree, relative to its top.
SOURCES = ["CMakeLists.txt", "LICENSE", "README.md", "CHANGELOG.md",
           "cmake/*", "lang/*", "src/*", "include/*", "schema/*"]


class SkywireConan(ConanFile):
    name = "skywire"
    description = ("Code generator for fixed-layout binary messages (wiregen), "
                   "with the C and C++ runtime headers")
    license = "Apache-2.0"
    url = "https://github.com/gatkinso/skywire"
    homepage = "https://github.com/gatkinso/skywire"
    topics = ("code-generator", "serialization", "binary", "icd", "wire-format")
    settings = "os", "arch", "compiler", "build_type"
    default_options = {"protobuf/*:shared": False}

    def _top(self):
        return os.path.join(self.recipe_folder, "..", "..")

    def set_version(self):
        cmakelists = load(self, os.path.join(self._top(), "CMakeLists.txt"))
        self.version = re.search(r"project\(skywire VERSION ([0-9.]+)\)", cmakelists).group(1)

    def export_sources(self):
        # src/test is only configured with BUILD_TESTING, which is off.
        for pattern in SOURCES:
            copy(self, pattern, self._top(), self.export_sources_folder,
                 excludes=["src/test/*", "*/.git", "*/.git/*"])

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("protobuf/6.33.5", visible=False)
        # Not "application": an application's requirements don't reach its
        # consumers, and generated C++ needs protozero's headers.
        self.requires("protozero/1.7.1", transitive_headers=True)

    def validate(self):
        check_min_cppstd(self, 17)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["BUILD_TESTING"] = False
        tc.cache_variables["SKYWIRE_EXAMPLES"] = False
        tc.cache_variables["SKYWIRE_INSTALL_DLLS"] = False
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))
        CMake(self).install()
        rmdir(self, os.path.join(self.package_folder, "share", "doc"))

    def package_info(self):
        # Consumers use the package's own skywireConfig.cmake, which also
        # defines skywire_generate(); it is found under the package root.
        self.cpp_info.set_property("cmake_find_mode", "none")
        self.cpp_info.builddirs = ["."]
        self.cpp_info.includedirs = [os.path.join("include", "skywire")]
        self.cpp_info.libdirs = []
        self.cpp_info.bindirs = ["bin"]
