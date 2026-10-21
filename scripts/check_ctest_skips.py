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
"""Fail when ctest skipped a test that a CI job expects to run.

The language round trips, the ring and the examples report Skipped when a
toolchain is missing, which keeps a local build green. In CI that would hide a
broken toolchain setup, so a job that installs every toolchain runs this on
ctest's JUnit output (ctest --output-junit) and fails on any skip.

    check_ctest_skips.py results.xml               # no test may be skipped
    check_ctest_skips.py results.xml --allow REGEX # except tests matching REGEX
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("junit", help="ctest --output-junit file")
    parser.add_argument("--allow", metavar="REGEX",
                        help="test names that may be skipped")
    args = parser.parse_args()

    allow = re.compile(args.allow) if args.allow else None
    cases = ET.parse(args.junit).getroot().iter("testcase")

    ran = 0
    skipped = []
    for case in cases:
        if case.find("skipped") is None:
            ran += 1
            continue
        name = case.get("name", "")
        if allow and allow.search(name):
            continue
        skipped.append((name, case.find("skipped").get("message", "")))

    if ran == 0:
        print(f"{args.junit}: no tests ran", file=sys.stderr)
        return 1

    for name, message in skipped:
        print(f"skipped: {name} ({message})", file=sys.stderr)
    if skipped:
        print(f"{len(skipped)} test(s) skipped; this job expects every "
              "toolchain to be installed", file=sys.stderr)
        return 1

    print(f"{ran} tests ran, none skipped")
    return 0


if __name__ == "__main__":
    sys.exit(main())
