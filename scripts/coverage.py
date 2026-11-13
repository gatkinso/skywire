#!/usr/bin/env python3
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
"""Report line and branch coverage of a -DSKYWIRE_COVERAGE=ON build.

    cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DSKYWIRE_COVERAGE=ON
    cmake --build build-cov && ctest --test-dir build-cov
    coverage.py build-cov                       # summary table
    coverage.py build-cov --html cov/index.html --xml cov/coverage.xml

Counts wiregen (src/lib, src/bin, include) and the C and C++ runtimes
(lang/c, lang/cpp), which the generated C/C++ tests exercise. Tests,
examples, benchmarks and generated code are left out.

Needs gcovr (pip install gcovr). Uses gcov for a GCC build and
llvm-cov gcov for a Clang one (xcrun llvm-cov gcov with Apple Clang).
--reset deletes the counters (.gcda files) so the next ctest starts from zero.
"""

import argparse
import glob
import os
import re
import shutil
import subprocess
import sys

SOURCE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Paths relative to the source tree, as gcovr regexes.
INCLUDE = [r"src/lib/", r"src/bin/", r"include/", r"lang/c/", r"lang/cpp/"]


def cache_value(build, name):
    path = os.path.join(build, "CMakeCache.txt")
    try:
        with open(path, encoding="utf-8") as f:
            for line in f:
                m = re.match(re.escape(name) + r"(?::[A-Z]+)?=(.*)$", line.rstrip("\n"))
                if m:
                    return m.group(1)
    except OSError:
        sys.exit(f"coverage.py: {path} not found; is {build} a build directory?")
    return ""


def compiler(build):
    """The C++ compiler's ID, path and version, from CMake's compiler file."""
    info = {}
    for path in glob.glob(os.path.join(build, "CMakeFiles", "*", "CMakeCXXCompiler.cmake")):
        with open(path, encoding="utf-8") as f:
            for m in re.finditer(r'^set\(CMAKE_CXX_COMPILER(|_ID|_VERSION) "([^"]*)"\)', f.read(), re.M):
                info[m.group(1)] = m.group(2)
    return info.get("_ID", ""), info.get("", ""), info.get("_VERSION", "")


def gcov_command(build):
    compiler_id, path, version = compiler(build)
    major = version.split(".")[0]
    if compiler_id == "GNU":
        # g++-13 writes counters for gcov-13.
        for name in (f"gcov-{major}", "gcov"):
            if shutil.which(name):
                return name
        sys.exit("coverage.py: gcov not found for this GCC build")
    if compiler_id == "AppleClang":
        return "xcrun llvm-cov gcov"
    if compiler_id == "Clang":
        # The llvm-cov beside clang, else e.g. clang-18's llvm-cov-18.
        beside = os.path.join(os.path.dirname(os.path.realpath(path)), "llvm-cov")
        if os.path.exists(beside):
            return f"{beside} gcov"
        for name in (f"llvm-cov-{major}", "llvm-cov"):
            if shutil.which(name):
                return f"{name} gcov"
        sys.exit("coverage.py: llvm-cov not found for this Clang build")
    sys.exit(f"coverage.py: {build} was built with {compiler_id or 'an unknown compiler'}; "
             "coverage needs GCC or Clang")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("build", help="build directory configured with -DSKYWIRE_COVERAGE=ON")
    parser.add_argument("--html", metavar="FILE", help="write an HTML report (one page per file)")
    parser.add_argument("--xml", metavar="FILE", help="write Cobertura XML (for Codecov and the like)")
    parser.add_argument("--reset", action="store_true", help="delete the counters and exit")
    args = parser.parse_args()

    build = os.path.abspath(args.build)
    if cache_value(build, "SKYWIRE_COVERAGE").upper() not in ("ON", "TRUE", "1", "YES"):
        sys.exit(f"coverage.py: {build} is not configured with -DSKYWIRE_COVERAGE=ON")

    gcda = glob.glob(os.path.join(build, "**", "*.gcda"), recursive=True)
    if args.reset:
        for path in gcda:
            os.remove(path)
        print(f"deleted {len(gcda)} .gcda files")
        return 0
    if not gcda:
        sys.exit(f"coverage.py: no .gcda files in {build}; run ctest first")
    if not shutil.which("gcovr"):
        sys.exit("coverage.py: gcovr not found (pip install gcovr)")

    cmd = ["gcovr", "--root", SOURCE, "--object-directory", build,
           "--gcov-executable", gcov_command(build),
           "-j", str(os.cpu_count() or 1),
           "--exclude-unreachable-branches", "--exclude-throw-branches",
           "--print-summary", "--txt", "-"]
    # Generated sources live in the build tree, outside these filters.
    for path in INCLUDE:
        cmd += ["--filter", re.escape(os.path.join(SOURCE, "")) + path]
    for out, flag in ((args.html, "--html-details"), (args.xml, "--cobertura")):
        if out:
            os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
            cmd += [flag, out]
    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
