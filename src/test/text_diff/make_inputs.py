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
"""Writes the inputs for skywire_text_diff: every file under the corpus
directories given, and COUNT seeded mutations of them (bytes changed,
inserted, deleted and spliced, and tokens of the three forms inserted), each
as a 4-byte little-endian length and the bytes.

    make_inputs.py <out> <count> <corpus dir>...
"""

import os
import random
import struct
import sys

TOKENS = [b'"', b"\\u00", b"\\ud800", b"\\udc00", b"e", b"E", b".", b"-", b"+", b"0", b"9", b"1e400",
          b"NaN", b"Infinity", b"<", b">", b"</", b"/>", b"&#x", b"&#", b"&amp;", b"&lt;", b"<![CDATA[",
          b"]]>", b"<!--", b"-->", b"<?", b"?>", b"<!DOCTYPE", b"\x00", b"\xff", b"\xc3\xa9", b"\xed\xa0\x80",
          b"\x10", b"\x12", b"\x13", b"\x01", b"\x02", b"\x03", b"\x05", b"\x05\x00\x00\x00\x00",
          b' encoding="hex"', b"\r", b"\r\n", b" ", b"\t", b"{", b"}", b"[", b"]", b",", b":", b"null",
          b"true", b"18446744073709551616", b"-9223372036854775809", b"1.5", b"3.4028236e38"]


def mutate(rng, data, seeds):
    data = bytearray(data)
    for _ in range(rng.randint(1, 4)):
        r = rng.random()
        i = rng.randint(0, len(data))
        if r < 0.25 and data:
            data[min(i, len(data) - 1)] = rng.randint(0, 255)
        elif r < 0.5:
            data[i:i] = rng.choice(TOKENS)
        elif r < 0.65 and data:
            del data[i:i + rng.randint(1, 8)]
        elif r < 0.85:
            other = rng.choice(seeds)
            j = rng.randint(0, len(other))
            data[i:i] = other[j:j + rng.randint(1, 40)]
        elif data:
            data[rng.randint(0, len(data) - 1)] = ord(rng.choice("0123456789"))
    return bytes(data)


def main():
    out, count, dirs = sys.argv[1], int(sys.argv[2]), sys.argv[3:]
    seeds = []
    for d in dirs:
        for name in sorted(os.listdir(d)):
            path = os.path.join(d, name)
            if os.path.isfile(path):
                with open(path, "rb") as f:
                    seeds.append(f.read())
    seeds = [s for s in seeds if s]
    rng = random.Random(20261111)
    inputs = seeds + [mutate(rng, rng.choice(seeds), seeds) for _ in range(count)]
    with open(out, "wb") as f:
        for data in inputs:
            f.write(struct.pack("<I", len(data)) + data)


if __name__ == "__main__":
    main()
