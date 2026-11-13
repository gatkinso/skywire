# Copyright (C) 2026 Geoffrey Atkinson
#
# Licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# A copy of the License is located at
#
# https://www.apache.org/licenses/LICENSE-2.0.html
#
# or in the "license" file accompanying this file. This file is distributed
# on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language governing
# permissions and limitations under the License.
"""Compares a language's skywire_text_diff output with the C++ oracle's, and
names the first inputs on which they differ.

    compare.py <inputs> <oracle output> <language output> <language>
"""

import struct
import sys


def main():
    inputs_path, want_path, got_path, language = sys.argv[1:5]
    with open(want_path) as f:
        want = f.read().split("\n")
    with open(got_path) as f:
        got = f.read().split("\n")
    with open(inputs_path, "rb") as f:
        raw = f.read()
    inputs, at = [], 0
    while at < len(raw):
        (n,) = struct.unpack_from("<I", raw, at)
        inputs.append(raw[at + 4:at + 4 + n])
        at += 4 + n

    accepted = {"J": 0, "X": 0, "B": 0}
    for line in want:
        if len(line) > 2 and line[2] != "-":
            accepted[line[0]] += 1
    if len(want) != len(got):
        print("%s printed %d lines, C++ %d" % (language, len(got), len(want)))
        return 1
    bad = [k for k, (w, g) in enumerate(zip(want, got)) if w != g]
    for k in bad[:5]:
        print("input %d: %r" % (k // 3, inputs[k // 3][:300]))
        print("  C++: %s" % want[k][:200])
        print("  %s: %s" % (language, got[k][:200]))
    print("%d inputs; C++ accepted %d as JSON, %d as XML, %d as BSON; %d differences"
          % (len(inputs), accepted["J"], accepted["X"], accepted["B"], len(bad)))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
