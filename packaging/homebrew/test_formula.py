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
"""Install the Homebrew formula from local release archives and test it.

    test_formula.py DIST --version 0.9.0 [--source REPO] [--no-audit] [--keep]

DIST holds the macOS arm64 release archive, as the release workflow makes it
or `gh release download` fetches it. The steps:

  1. make_formula.py writes the formula, with a file:// URL into DIST and
     the archive's checksum, into a scratch tap, skywire-test/local, made
     with `brew tap-new --no-git`.
  2. brew installs it (and protozero), then runs `brew test` and, unless
     --no-audit, `brew audit` on it. (The first audit installs Homebrew's
     Ruby gems, which takes a minute.)
  3. src/test/install/consumer, configured with the Homebrew prefix as
     CMAKE_PREFIX_PATH, builds and each program writes examples/Telemetry.bin.
  4. The formula is uninstalled (brew autoremoves protozero if nothing else
     needs it) and the tap removed, unless --keep.

skywire must not already be installed from another tap.
"""

import argparse
import filecmp
import glob
import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

TAP = "skywire-test/local"
FORMULA = TAP + "/skywire"
HERE = os.path.dirname(os.path.abspath(__file__))


def run(cmd, **kwargs):
    print("+ " + " ".join(cmd), flush=True)
    return subprocess.run(cmd, check=True, text=True, **kwargs)


def output(cmd):
    return run(cmd, stdout=subprocess.PIPE).stdout.strip()


def fail(message):
    print("FAILED: " + message, file=sys.stderr)
    sys.exit(1)


def write_sums(dist, path):
    with open(path, "w") as f:
        for archive in sorted(glob.glob(os.path.join(dist, "*.tar.gz"))):
            with open(archive, "rb") as a:
                sha = hashlib.sha256(a.read()).hexdigest()
            f.write(f"{sha}  {os.path.basename(archive)}\n")


def developer_mode():
    return "enabled" in output(["brew", "developer"])


def make_tap(dist, version, workdir):
    if subprocess.run(["brew", "list", "--formula", "skywire"],
                      capture_output=True).returncode == 0:
        fail("skywire is already installed; brew uninstall it first")
    if subprocess.run(["brew", "tap-info", TAP], capture_output=True,
                      text=True).stdout.find("Not installed") == -1:
        run(["brew", "untap", "--force", TAP])
    run(["brew", "tap-new", "--no-git", TAP])
    tap_dir = output(["brew", "--repository", TAP])
    formula = os.path.join(tap_dir, "Formula", "skywire.rb")
    sums = os.path.join(workdir, "SHA256SUMS")
    write_sums(dist, sums)
    run([sys.executable, os.path.join(HERE, "make_formula.py"), version, sums,
         "--url-base", "file://" + dist, "-o", formula])
    return formula


def build_consumer(source, workdir):
    brew_prefix = output(["brew", "--prefix"])
    examples = os.path.join(source, "examples")
    build = os.path.join(workdir, "consumer")
    env = dict(os.environ,
               PKG_CONFIG_PATH=os.path.join(brew_prefix, "lib", "pkgconfig"))
    run(["cmake", "-S", os.path.join(source, "src", "test", "install", "consumer"),
         "-B", build, "-DCMAKE_BUILD_TYPE=Release",
         "-DCMAKE_PREFIX_PATH=" + brew_prefix, "-DEXAMPLES_DIR=" + examples],
        env=env)
    run(["cmake", "--build", build], env=env)
    programs = sorted(glob.glob(os.path.join(build, "consumer_*")))
    programs = [p for p in programs if os.access(p, os.X_OK) and os.path.isfile(p)]
    if len(programs) < 2:
        fail(f"expected consumer_c and consumer_cpp, built {programs}")
    for program in programs:
        out = os.path.join(workdir, os.path.basename(program) + ".bin")
        run([program, out])
        if not filecmp.cmp(out, os.path.join(examples, "Telemetry.bin"), shallow=False):
            fail(f"{os.path.basename(program)} wrote different bytes than Telemetry.bin")
    print("consumer built against the Homebrew install: "
          + ", ".join(os.path.basename(p) for p in programs))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("dist", help="the directory holding the release archive")
    parser.add_argument("--version", required=True,
                        help="the archive's version, e.g. 0.9.0 or 0.10.0-rc1")
    parser.add_argument("--source", default=os.path.dirname(os.path.dirname(HERE)),
                        help="the skywire source tree (default: this script's)")
    parser.add_argument("--no-audit", action="store_true",
                        help="skip brew audit")
    parser.add_argument("--keep", action="store_true",
                        help="leave the formula installed and the tap in place")
    args = parser.parse_args()

    if shutil.which("brew") is None:
        fail("brew is not on PATH")
    os.environ.update(HOMEBREW_NO_AUTO_UPDATE="1", HOMEBREW_NO_INSTALL_CLEANUP="1",
                      HOMEBREW_NO_ANALYTICS="1", HOMEBREW_NO_ENV_HINTS="1")
    dist = os.path.abspath(args.dist)

    # brew tap-new turns on developer mode; put it back afterwards.
    was_developer = developer_mode()
    workdir = tempfile.mkdtemp(prefix="skywire-brew-")
    formula = make_tap(dist, args.version, workdir)
    try:
        run(["brew", "install", "--formula", FORMULA])
        run(["brew", "test", FORMULA])
        if not args.no_audit:
            run(["brew", "audit", "--formula", FORMULA])
        build_consumer(os.path.abspath(args.source), workdir)
    finally:
        shutil.rmtree(workdir, ignore_errors=True)
        if args.keep:
            print(f"kept: {FORMULA} installed, formula at {formula}")
        else:
            subprocess.run(["brew", "uninstall", "--formula", FORMULA])
            subprocess.run(["brew", "untap", "--force", TAP])
        if not was_developer:
            subprocess.run(["brew", "developer", "off"])
    print("Homebrew formula OK")


if __name__ == "__main__":
    main()
