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
"""Install the vcpkg overlay port in manifest mode and build a project with it.

    test_port.py [--vcpkg-root DIR] [--triplet T] [--host-triplet T]
                 [--source REPO] [--keep]

The steps:

  1. The port's version must be the project's (project() in CMakeLists.txt).
  2. A copy of the port, with SOURCE_PATH set to the source tree and a hash
     of the tree in a comment, goes into a scratch overlay directory. The
     hash changes the port's ABI hash, so vcpkg's binary cache never hands
     back a skywire built from different sources.
  3. src/test/install/consumer is configured with the vcpkg toolchain and a
     scratch manifest that depends on skywire (with the baseline of
     .github/vcpkg/vcpkg.json), so vcpkg installs the port and its
     dependencies into the scratch directory. The consumer builds, and each
     program writes examples/Telemetry.bin.
  4. The install has wiregen in tools/skywire, not bin, and the copyright
     and usage files.

DIR is the vcpkg root (default: $VCPKG_ROOT); its checkout must contain the
baseline commit. The scratch directory is removed unless --keep.
"""

import argparse
import filecmp
import glob
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.join(HERE, "skywire")

# What the port builds from; the hash covers these.
SOURCES = ["CMakeLists.txt", "LICENSE", "cmake", "lang", "src", "include", "3rd_party"]

SOURCE_PATH_LINE = ('get_filename_component(SOURCE_PATH '
                    '"${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)')


def run(cmd, **kwargs):
    print("+ " + " ".join(cmd), flush=True)
    return subprocess.run(cmd, check=True, text=True, **kwargs)


def fail(message):
    print("FAILED: " + message, file=sys.stderr)
    sys.exit(1)


def check_version(source):
    with open(os.path.join(source, "CMakeLists.txt")) as f:
        project = re.search(r"project\(skywire VERSION ([0-9.]+)\)", f.read()).group(1)
    with open(os.path.join(PORT, "vcpkg.json")) as f:
        port = json.load(f)["version"]
    if port != project:
        fail(f"packaging/vcpkg/skywire/vcpkg.json has version {port}, "
             f"CMakeLists.txt has {project}")
    print(f"port version {port} matches the project")


def tree_hash(source):
    sha = hashlib.sha256()
    for top in SOURCES:
        path = os.path.join(source, top)
        files = [path] if os.path.isfile(path) else sorted(
            os.path.join(d, f) for d, dirs, names in os.walk(path)
            if ".git" not in d.split(os.sep) for f in names if f != ".git")
        for file in files:
            sha.update(os.path.relpath(file, source).replace(os.sep, "/").encode())
            with open(file, "rb") as f:
                sha.update(hashlib.sha256(f.read()).digest())
    return sha.hexdigest()


def stage_port(source, overlay):
    port = os.path.join(overlay, "skywire")
    os.makedirs(port)
    for name in ("vcpkg.json", "usage"):
        shutil.copy(os.path.join(PORT, name), port)
    with open(os.path.join(PORT, "portfile.cmake")) as f:
        portfile = f.read()
    if SOURCE_PATH_LINE not in portfile:
        fail("portfile.cmake no longer sets SOURCE_PATH the way this script expects")
    cmake_source = source.replace("\\", "/")
    portfile = portfile.replace(
        SOURCE_PATH_LINE,
        f'set(SOURCE_PATH "{cmake_source}")\n# Source tree hash: {tree_hash(source)}')
    with open(os.path.join(port, "portfile.cmake"), "w") as f:
        f.write(portfile)


def write_manifest(source, manifest):
    with open(os.path.join(source, ".github", "vcpkg", "vcpkg.json")) as f:
        baseline = json.load(f)["builtin-baseline"]
    os.makedirs(manifest)
    with open(os.path.join(manifest, "vcpkg.json"), "w") as f:
        json.dump({"name": "skywire-port-test", "version-string": "0",
                   "builtin-baseline": baseline, "dependencies": ["skywire"]},
                  f, indent=2)


def build_consumer(args, source, workdir):
    examples = os.path.join(source, "examples")
    build = os.path.join(workdir, "build")
    installed = os.path.join(workdir, "installed")
    configure = [
        "cmake", "-S", os.path.join(source, "src", "test", "install", "consumer"),
        "-B", build, "-DCMAKE_BUILD_TYPE=Release",
        "-DCMAKE_TOOLCHAIN_FILE=" + os.path.join(
            args.vcpkg_root, "scripts", "buildsystems", "vcpkg.cmake"),
        "-DVCPKG_MANIFEST_DIR=" + os.path.join(workdir, "manifest"),
        "-DVCPKG_INSTALLED_DIR=" + installed,
        "-DVCPKG_OVERLAY_PORTS=" + os.path.join(workdir, "overlay"),
        "-DEXAMPLES_DIR=" + examples]
    if args.triplet:
        configure.append("-DVCPKG_TARGET_TRIPLET=" + args.triplet)
    if args.host_triplet:
        configure.append("-DVCPKG_HOST_TRIPLET=" + args.host_triplet)
    run(configure)
    run(["cmake", "--build", build, "--config", "Release"])

    exe = ".exe" if sys.platform == "win32" else ""
    programs = sorted(p for p in glob.glob(os.path.join(build, "**", "consumer_*" + exe),
                                           recursive=True)
                      if os.path.isfile(p) and os.access(p, os.X_OK))
    names = [os.path.basename(p) for p in programs]
    if "consumer_c" + exe not in names or "consumer_cpp" + exe not in names:
        fail(f"expected consumer_c and consumer_cpp, built {names}")
    for program in programs:
        out = os.path.join(workdir, os.path.basename(program) + ".bin")
        run([program, out])
        if not filecmp.cmp(out, os.path.join(examples, "Telemetry.bin"), shallow=False):
            fail(f"{os.path.basename(program)} wrote different bytes than Telemetry.bin")
    print("consumer built with the vcpkg port: " + ", ".join(names))
    return installed


def check_layout(installed):
    prefixes = glob.glob(os.path.join(installed, "*", "share", "skywire", "copyright"))
    if len(prefixes) != 1:
        fail(f"expected one share/skywire/copyright under {installed}, found {prefixes}")
    prefix = os.path.dirname(os.path.dirname(os.path.dirname(prefixes[0])))
    exe = ".exe" if sys.platform == "win32" else ""
    if not os.path.isfile(os.path.join(prefix, "tools", "skywire", "wiregen" + exe)):
        fail("no tools/skywire/wiregen")
    if not os.path.isfile(os.path.join(prefix, "share", "skywire", "usage")):
        fail("no share/skywire/usage")
    if glob.glob(os.path.join(prefix, "bin", "wiregen*")):
        fail("wiregen installed in bin")
    out = run([os.path.join(prefix, "tools", "skywire", "wiregen" + exe), "--version"],
              stdout=subprocess.PIPE).stdout.strip()
    print(out)
    print(f"layout OK in {prefix}")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--vcpkg-root", default=os.environ.get("VCPKG_ROOT"),
                        help="the vcpkg checkout (default: $VCPKG_ROOT)")
    parser.add_argument("--triplet", help="the target triplet (default: vcpkg's)")
    parser.add_argument("--host-triplet", help="the host triplet (default: vcpkg's)")
    parser.add_argument("--source", default=os.path.dirname(os.path.dirname(HERE)),
                        help="the skywire source tree (default: this script's)")
    parser.add_argument("--keep", action="store_true",
                        help="keep the scratch directory")
    args = parser.parse_args()

    if not args.vcpkg_root:
        fail("set VCPKG_ROOT or pass --vcpkg-root")
    args.vcpkg_root = os.path.abspath(args.vcpkg_root)
    source = os.path.abspath(args.source)
    check_version(source)

    workdir = tempfile.mkdtemp(prefix="skywire-vcpkg-")
    try:
        stage_port(source, os.path.join(workdir, "overlay"))
        write_manifest(source, os.path.join(workdir, "manifest"))
        installed = build_consumer(args, source, workdir)
        check_layout(installed)
    finally:
        if args.keep:
            print("kept " + workdir)
        else:
            shutil.rmtree(workdir, ignore_errors=True)
    print("vcpkg port OK")


if __name__ == "__main__":
    main()
