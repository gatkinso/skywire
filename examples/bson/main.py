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

"""Reads a Telemetry from a BSON document as a MongoDB driver might hand it
back (telemetry.bson: an _id and a received_at the message does not have,
members in another order, numbers in other types), writes it back in the
canonical form, shows a uint64_t above 2^63-1 written as a Decimal128, and
shows what from_bson refuses. See README.md.

    python3 main.py telemetry.bson out.bson
"""

import copy
import sys

from Telemetry import Telemetry

UINT64_MAX = (1 << 64) - 1


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: main.py INPUT OUTPUT")

    with open(sys.argv[1], "rb") as f:
        doc = f.read()

    # from_bson ignores elements the message does not have (_id,
    # received_at) and takes any number type that holds the member's value:
    # here an int32 device_id and a double battery_mv.
    msg = Telemetry()
    if not msg.from_bson(doc):
        sys.exit("%s is not the BSON form of a Telemetry" % sys.argv[1])
    print("device_id %d, callsign %s, battery_mv %d" % (
        msg.Get_device_id(), msg.Get_callsign().rstrip(b"\0").decode(), msg.Get_battery_mv()))

    # to_bson writes the canonical document: members in tag order, each in
    # the one type its member's type maps to (uint32_t device_id as int64,
    # uint16_t battery_mv as int32).
    canonical = msg.to_bson()
    with open(sys.argv[2], "wb") as f:
        f.write(canonical)
    print("read %d bytes, wrote %d" % (len(doc), len(canonical)))

    # A uint64_t above 2^63-1 is written as a Decimal128 (type 0x13), so no
    # tool reads it as a negative int64.
    big = copy.deepcopy(msg)
    big.Set_timestamp_us(UINT64_MAX)
    big_doc = big.to_bson()
    back = Telemetry()
    if b"\x13timestamp_us\0" not in big_doc or not back.from_bson(big_doc) or back != big:
        sys.exit("timestamp_us %d did not go through as a Decimal128" % UINT64_MAX)
    print("timestamp_us %d is a Decimal128, and reads back" % UINT64_MAX)

    # A document that is not a Telemetry, or holds a value a member cannot,
    # is refused, and the message is left as it was.
    refused = [
        (doc[:-1], "cut short"),
        (doc + b"\0", "a byte after the document"),
        (b"\x11\0\0\0\x10status\0\x09\0\0\0\0", "{status: int32 9}: status has 3 bits"),
        (b"\x18\0\0\0\x12device_id\0" + b"\xff" * 8 + b"\0", "{device_id: int64 -1}: a uint32_t"),
        (b"\x1b\0\0\0\x01timestamp_us\0\0\0\0\0\0\0\xf8\x3f\0",
         "{timestamp_us: double 1.5}: not a whole number"),
    ]
    for bad, why in refused:
        before = copy.deepcopy(msg)
        if msg.from_bson(bad) or msg != before:
            sys.exit("from_bson accepted a document: %s" % why)
        print("refused %s" % why)


if __name__ == "__main__":
    main()
