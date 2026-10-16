#!/usr/bin/env python3
#
#  Copyright (C) 2019 Skyloupe, LLC
#
#  Licensed under the Apache License, Version 2.0 (the "License").
#  You may not use this file except in compliance with the License.
#  A copy of the License is located at
#
#  https://www.apache.org/licenses/LICENSE-2.0.html
#
#  or in the "license" file accompanying this file. This file is distributed
#  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#  express or implied. See the License for the specific language governing
#  permissions and limitations under the License.
#

"""Write RingyMcRingface.bin, the input file of the cross-language serial test.

The bytes are encoded here with Python's struct module, independently of
wiregen and of every generated language, so the file is a reference that
every generator is checked against. See scripts/README.md.
"""

import argparse
import json
import struct
import sys
from pathlib import Path

RING_DIR = Path(__file__).resolve().parent.parent / "src" / "test" / "generated_tests" / "ring"

# Test value of each type: (struct code, value). The values MUST equal the
# literals in src/lib/skywire/test_values.cpp, which every relay checks against.
TYPES = {
    "char":          ("b", 65),
    "unsigned char": ("B", 200),
    "uint8_t":       ("B", 201),
    "int8_t":        ("b", -50),
    "uint16_t":      ("H", 60000),
    "int16_t":       ("h", -12345),
    "uint32_t":      ("I", 3000000000),
    "int32_t":       ("i", -2000000000),
    "uint64_t":      ("Q", 12000000000),
    "int64_t":       ("q", -9000000000),
    "float":         ("f", 3.5),
    "double":        ("d", 3.14159),
}

# test_values::string_value(), truncated or zero-padded to the member length.
STRING_VALUE = b"skywire"

BYTE_ORDER = {0: ">", 1: "<"}


def load_spec(path):
    """Parse the ring spec."""
    return json.loads(path.read_text())


def encode(spec, spec_dir, prefix="", offset=0):
    """Return (wire bytes, README table rows) for the members in tag order.

    A member whose type is another message is that message's own encoding,
    inline, read from <type>.json in spec_dir; its rows name the member by
    its path (outer.inner).
    """
    wire = b""
    rows = []
    for tag, member in sorted(spec["members"].items(), key=lambda item: int(item[0])):
        name, mtype = prefix + member["name"], member["type"]
        endian = member.get("endian", 0)

        if mtype == "string":
            length = member["length"]
            text = STRING_VALUE[:length]
            data = text.ljust(length, b"\0")
            shown = f'`{name}` [{length}] "{text.decode()}"'
            pad = length - len(text)
            if pad:
                hexed = f"`{text.hex(' ')}` + {pad} × `00`"
            elif length == len(STRING_VALUE):
                hexed = f"`{data.hex(' ')}` (no terminator)"
            else:
                hexed = f"`{data.hex(' ')}`"
        elif mtype in TYPES:
            code, value = TYPES[mtype]
            if endian not in BYTE_ORDER:
                sys.exit(f"member {tag} ({name}): endian {endian} is host-dependent and has no fixed wire bytes")
            data = struct.pack(BYTE_ORDER[endian] + code, value)
            shown = f"`{name}` {mtype} {value}" if struct.calcsize(code) == 1 else f"`{name}` {value}"
            hexed = f"`{data.hex(' ')}`"
        else:
            nested = spec_dir / f"{mtype}.json"
            if not nested.exists():
                sys.exit(f"member {tag} ({name}): type '{mtype}' is not a known type, and {nested} does not exist; "
                         f"add the type to TYPES in {Path(__file__).name} or write its spec")
            data, nested_rows = encode(load_spec(nested), spec_dir, name + ".", offset + len(wire))
            rows += nested_rows
            wire += data
            continue

        rows.append(f"| {offset + len(wire)} | {shown} | {hexed} |")
        wire += data
    return wire, rows


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--spec", type=Path, default=RING_DIR / "RingyMcRingface.json",
                        help="spec to read (default: %(default)s)")
    parser.add_argument("--out", type=Path, default=RING_DIR / "RingyMcRingface.bin",
                        help="file to write (default: %(default)s)")
    parser.add_argument("--check", action="store_true",
                        help="only check that --out is up to date; exit 1 if it is not")
    args = parser.parse_args()

    wire, rows = encode(load_spec(args.spec), args.spec.parent)

    print("| Offset | Member | Bytes |")
    print("|---:|---|---|")
    print("\n".join(rows))
    print(f"\n{len(wire)} bytes")

    if args.check:
        current = args.out.read_bytes() if args.out.exists() else None
        if current != wire:
            print(f"{args.out} is out of date; run {Path(__file__).name} without --check", file=sys.stderr)
            return 1
        print(f"{args.out} is up to date")
        return 0

    args.out.write_bytes(wire)
    print(f"wrote {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
