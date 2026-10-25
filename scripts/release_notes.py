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
"""Check a release tag against the source and print its release notes.

    release_notes.py v0.9.0       # notes for 0.9.0, from CHANGELOG.md
    release_notes.py v0.9.0-rc1   # a pre-release of 0.9.0: the same notes

The tag must be v<major>.<minor>.<patch>, optionally followed by a
pre-release suffix (-rc1, -beta.2). The checks:

  * project(skywire VERSION ...) in CMakeLists.txt is the tag's version, so
    wiregen --version and the archive names agree with the tag.
  * CHANGELOG.md has a "## [<version>]" section, which becomes the notes.
  * For a release (no suffix) the section is dated, "## [0.9.0] - 2026-11-02",
    not "Unreleased": date the CHANGELOG, then tag.
"""

import argparse
import os
import re
import sys

TAG = re.compile(r"^v(\d+\.\d+\.\d+)(-[0-9A-Za-z.-]+)?$")


def fail(message):
    print(f"release_notes.py: {message}", file=sys.stderr)
    sys.exit(1)


def project_version(cmakelists):
    with open(cmakelists, encoding="utf-8") as f:
        match = re.search(r"project\(\s*skywire\s+VERSION\s+([0-9.]+)", f.read())
    if not match:
        fail(f"no project(skywire VERSION ...) in {cmakelists}")
    return match.group(1)


def changelog_section(changelog, version):
    """Returns the section's heading line and its body, without link lines."""
    with open(changelog, encoding="utf-8") as f:
        lines = f.read().splitlines()
    start = next((i for i, line in enumerate(lines)
                  if line.startswith(f"## [{version}]")), None)
    if start is None:
        fail(f"{changelog} has no '## [{version}]' section")
    end = next((i for i in range(start + 1, len(lines))
                if lines[i].startswith("## [")
                or re.match(r"^\[[^\]]+\]: ", lines[i])), len(lines))
    return lines[start], "\n".join(lines[start + 1:end]).strip() + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("tag", help="the release tag, e.g. v0.9.0")
    parser.add_argument("--source", default=os.path.join(os.path.dirname(
                        os.path.abspath(__file__)), os.pardir),
                        help="skywire source tree (default: this script's)")
    args = parser.parse_args()

    match = TAG.match(args.tag)
    if not match:
        fail(f"{args.tag!r} is not v<major>.<minor>.<patch>[-<pre-release>]")
    version, prerelease = match.group(1), match.group(2)

    project = project_version(os.path.join(args.source, "CMakeLists.txt"))
    if project != version:
        fail(f"tag {args.tag} is version {version}, but CMakeLists.txt says "
             f"project(skywire VERSION {project})")

    heading, notes = changelog_section(
        os.path.join(args.source, "CHANGELOG.md"), version)
    if not prerelease and not re.search(r"\d{4}-\d{2}-\d{2}", heading):
        fail(f"'{heading}' has no release date; date it before tagging "
             f"{args.tag}")

    sys.stdout.write(notes)
    return 0


if __name__ == "__main__":
    sys.exit(main())
