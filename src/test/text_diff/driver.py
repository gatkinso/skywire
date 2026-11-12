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
"""The Python driver for skywire_text_diff: prints what the generated Python
ring message makes of each input, as oracle.cpp does for C++.

    driver.py <generated dir> <inputs>
"""

import struct
import sys


def main():
    sys.path.insert(0, sys.argv[1])
    from RingyMcRingface import RingyMcRingface

    with open(sys.argv[2], "rb") as f:
        raw = f.read()
    out, at = [], 0
    while at < len(raw):
        (n,) = struct.unpack_from("<I", raw, at)
        data = raw[at + 4:at + 4 + n]
        at += 4 + n
        m = RingyMcRingface()
        out.append("J " + m.to_json().encode().hex() if m.from_json(data) else "J -")
        m = RingyMcRingface()
        out.append("X " + m.to_xml().encode().hex() if m.from_xml(data) else "X -")
        m = RingyMcRingface()
        out.append("B " + m.to_bson().hex() if m.from_bson(data) else "B -")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
