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

"""Writes telemetry.bson, a Telemetry document as a MongoDB driver might hand
it back: an _id and a received_at the message does not have, the members in
another order, and numbers in the types the driver chose (an int32
device_id, a double battery_mv) rather than the ones to_bson writes. Python's
struct only, so the file does not depend on any skywire code.

    python3 make_input.py [telemetry.bson]
"""

import struct
import sys


def element(kind, name, payload):
    return bytes([kind]) + name.encode() + b"\0" + payload


def document(*elements):
    body = b"".join(elements) + b"\0"
    return struct.pack("<i", len(body) + 4) + body


def double(name, v):
    return element(0x01, name, struct.pack("<d", v))


def string(name, s):
    data = s.encode() + b"\0"
    return element(0x02, name, struct.pack("<i", len(data)) + data)


def int32(name, v):
    return element(0x10, name, struct.pack("<i", v))


def int64(name, v):
    return element(0x12, name, struct.pack("<q", v))


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "telemetry.bson"
    doc = document(
        element(0x07, "_id", bytes.fromhex("6735a1c2e4b0a1b2c3d4e5f6")),  # ObjectId
        int32("device_id", 4207),
        string("callsign", "SKY1"),
        int64("timestamp_us", 1760959800123456),
        element(0x03, "position", document(
            double("latitude", 47.6062),
            double("longitude", -122.3321),
            double("altitude_m", 56.5),
        )),
        int32("mode", 5),
        int32("status", 3),
        int32("armed", 1),
        double("battery_mv", 3870.0),
        element(0x09, "received_at", struct.pack("<q", 1760959800123)),  # UTC datetime
    )
    with open(path, "wb") as f:
        f.write(doc)


if __name__ == "__main__":
    main()
