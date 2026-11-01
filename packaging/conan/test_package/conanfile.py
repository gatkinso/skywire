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
"""Builds src/test/install/consumer against the package (find_package,
skywire_generate() and, with pkg-config, skywire.pc), and checks that each
program writes examples/Telemetry.bin."""

import filecmp
import glob
import os

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.build import can_run
from conan.tools.cmake import CMake, cmake_layout


class SkywireTestConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain"

    def _top(self):
        return os.path.abspath(os.path.join(self.recipe_folder, "..", "..", ".."))

    def requirements(self):
        self.requires(self.tested_reference_str)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure(
            build_script_folder=os.path.join(self._top(), "src", "test", "install", "consumer"),
            variables={"EXAMPLES_DIR": os.path.join(self._top(), "examples").replace("\\", "/")})
        cmake.build()

    def test(self):
        if not can_run(self):
            return
        exe = ".exe" if self.settings.os == "Windows" else ""
        programs = sorted(p for p in glob.glob(os.path.join(self.build_folder, "**", "consumer_*" + exe),
                                               recursive=True)
                          if os.path.isfile(p) and os.access(p, os.X_OK))
        names = [os.path.basename(p) for p in programs]
        if "consumer_c" + exe not in names or "consumer_cpp" + exe not in names:
            raise ConanException(f"expected consumer_c and consumer_cpp, built {names}")
        expected = os.path.join(self._top(), "examples", "Telemetry.bin")
        for program in programs:
            out = os.path.join(self.build_folder, os.path.basename(program) + ".bin")
            self.run(f'"{program}" "{out}"', env="conanrun")
            if not filecmp.cmp(out, expected, shallow=False):
                raise ConanException(f"{os.path.basename(program)} wrote different bytes than Telemetry.bin")
        self.output.info("consumer built with the Conan package: " + ", ".join(names))
