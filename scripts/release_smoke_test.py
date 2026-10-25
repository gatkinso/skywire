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
"""Check an unpacked skywire release archive on a machine that never built it.

    release_smoke_test.py PREFIX --version 0.9.0 [--source REPO]

PREFIX is the unpacked archive (it holds bin/, include/, share/). The checks:

  1. wiregen --version prints the expected version, and --banner runs.
  2. wiregen needs only system libraries: no protobuf, abseil or C++ runtime
     to install (ldd, otool -L or dumpbin /dependents).
  3. wiregen generates every language from the installed lang files.
  4. The generated C, compiled against the installed runtime headers, writes
     the same bytes as examples/Telemetry.bin.

The C compiler is $CC or cc, or cl on Windows (run it from a developer shell,
which dumpbin also needs).
"""

import argparse
import filecmp
import os
import re
import shutil
import subprocess
import sys
import tempfile

LANGUAGES = ["c", "cpp", "go", "java", "proto3", "python3", "rust", "swift"]
SPECS = ["Position.json", "Telemetry.json"]

# Libraries every machine has. Anything else wiregen needs would have to be
# installed alongside it.
LINUX_SYSTEM = re.compile(
    r"^(linux-vdso|linux-gate|ld-linux[^ ]*|libc|libm|libpthread|libdl|librt)\.so")
MACOS_SYSTEM = re.compile(r"^(/usr/lib/|/System/Library/)")
# In System32 on the runner, but only because a Visual C++ redistributable is
# installed there; a user's machine may not have them.
WINDOWS_REDIST = re.compile(r"^(vcruntime|msvcp|concrt|vccorlib|ucrtbased)",
                            re.IGNORECASE)


def run(cmd, **kwargs):
    print("+ " + " ".join(cmd))
    return subprocess.run(cmd, check=True, text=True, **kwargs)


def output(cmd):
    return run(cmd, stdout=subprocess.PIPE).stdout


def fail(message):
    print("FAILED: " + message, file=sys.stderr)
    sys.exit(1)


def check_version(wiregen, version):
    out = output([wiregen, "--version"]).strip()
    print(out)
    if out != "wiregen " + version:
        fail(f"wiregen --version printed {out!r}, expected 'wiregen {version}'")
    print(output([wiregen, "--banner"]))


def dependencies(wiregen):
    """Returns the libraries wiregen loads that a user would have to install."""
    if sys.platform.startswith("linux"):
        libs = [line.split()[0] for line in output(["ldd", wiregen]).splitlines()
                if line.strip()]
        return [lib for lib in libs
                if not LINUX_SYSTEM.match(os.path.basename(lib))]
    if sys.platform == "darwin":
        lines = output(["otool", "-L", wiregen]).splitlines()[1:]
        libs = [line.split()[0] for line in lines if line.strip()]
        return [lib for lib in libs if not MACOS_SYSTEM.match(lib)]
    if sys.platform == "win32":
        out = output(["dumpbin", "/nologo", "/dependents", wiregen])
        libs = re.findall(r"^\s+(\S+\.dll)\s*$", out, re.MULTILINE | re.IGNORECASE)
        system32 = os.path.join(os.environ["SystemRoot"], "System32")
        return [lib for lib in libs
                if WINDOWS_REDIST.match(lib)
                or not (lib.lower().startswith("api-ms-")
                        or os.path.exists(os.path.join(system32, lib)))]
    fail(f"no dependency check for {sys.platform}")


def check_dependencies(wiregen, prefix):
    extra = dependencies(wiregen)
    if extra:
        fail("wiregen needs libraries that are not part of the system: "
             + ", ".join(extra))
    exe = os.path.basename(wiregen)
    others = sorted(set(os.listdir(os.path.join(prefix, "bin"))) - {exe})
    if others:
        fail("bin/ holds more than wiregen: " + ", ".join(others))
    print("wiregen needs only system libraries")


def generate(wiregen, specs_dir, language, outdir):
    os.makedirs(outdir)
    for spec in SPECS:
        run([wiregen, "--language", language, "--write_to_file", "--silent",
             "--input_file", os.path.join(specs_dir, spec), "--outdir", outdir])
    if not os.listdir(outdir):
        fail(f"wiregen --language {language} wrote nothing")


def compile_c(gen, include, main_c, exe):
    sources = [main_c] + sorted(os.path.join(gen, f)
                                for f in os.listdir(gen) if f.endswith(".c"))
    if sys.platform == "win32":
        run(["cl", "/nologo", "/W3", f"/I{gen}", f"/I{include}",
             f"/Fo{gen}\\", f"/Fe{exe}"] + sources)
    else:
        cc = os.environ.get("CC", "cc")
        run([cc, "-std=c99", "-Wall", "-I" + gen, "-I" + include, "-o", exe]
            + sources)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("prefix", help="the unpacked release archive")
    parser.add_argument("--version", required=True,
                        help="the version wiregen --version must print")
    parser.add_argument("--source", default=os.path.join(os.path.dirname(
                        os.path.abspath(__file__)), os.pardir),
                        help="skywire source tree, for examples/ (default: "
                             "this script's)")
    args = parser.parse_args()
    sys.stdout.reconfigure(line_buffering=True)   # keep order with stderr in CI logs

    prefix = os.path.abspath(args.prefix)
    examples = os.path.join(os.path.abspath(args.source), "examples")
    exe = ".exe" if sys.platform == "win32" else ""
    wiregen = os.path.join(prefix, "bin", "wiregen" + exe)
    if not os.path.isfile(wiregen):
        fail(f"no wiregen in {prefix}/bin")

    check_version(wiregen, args.version)
    check_dependencies(wiregen, prefix)

    work = tempfile.mkdtemp(prefix="skywire-smoke-")
    try:
        for language in LANGUAGES:
            generate(wiregen, os.path.join(examples, "specs"), language,
                     os.path.join(work, language))
        print(f"generated {', '.join(LANGUAGES)}")

        program = os.path.join(work, "example_c" + exe)
        compile_c(os.path.join(work, "c"),
                  os.path.join(prefix, "include", "skywire"),
                  os.path.join(examples, "c", "main.c"), program)
        written = os.path.join(work, "telemetry.bin")
        run([program, written])
        if not filecmp.cmp(written, os.path.join(examples, "Telemetry.bin"),
                           shallow=False):
            fail("the C example's output differs from examples/Telemetry.bin")
        print("generated C compiles and writes examples/Telemetry.bin")
    finally:
        shutil.rmtree(work, ignore_errors=True)

    print("release smoke test passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
