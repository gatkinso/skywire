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

"""Reads a Telemetry from JSON written by hand (telemetry.json: indented,
members in another order, one missing and one the message does not have),
writes it back in the canonical form, and shows what from_json refuses. See
README.md.

    python3 main.py telemetry.json out.json
"""

import copy
import sys

from Telemetry import Telemetry


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: main.py INPUT OUTPUT")

    with open(sys.argv[1], "rb") as f:
        text = f.read()

    # from_json takes members in any order, leaves a missing one (battery_mv)
    # zero and ignores one the message does not have (firmware). A 64-bit
    # integer may be a JSON string, as to_json writes it.
    msg = Telemetry()
    if not msg.from_json(text):
        sys.exit("%s is not the JSON form of a Telemetry" % sys.argv[1])
    print("device_id %d, callsign %s, timestamp_us %d, battery_mv %d" % (
        msg.Get_device_id(), msg.Get_callsign().rstrip(b"\0").decode(),
        msg.Get_timestamp_us(), msg.Get_battery_mv()))

    # to_json writes the canonical form: no whitespace, members in tag order.
    canonical = msg.to_json()
    with open(sys.argv[2], "w", encoding="utf-8", newline="") as f:
        f.write(canonical)
    print(canonical)

    # Input that is not a Telemetry, or holds a value a member cannot, is
    # refused, and the message is left as it was.
    refused = [
        '{"status": 9}',                      # status has 3 bits
        '{"device_id": -1}',                  # a uint32_t
        '{"timestamp_us": 1.5}',              # not a whole number
        '{"callsign": "TOO-LONG-CALLSIGN"}',  # more than 8 bytes
        '{"armed": true}',                    # not a number
        '{"mode": 5,}',                       # not JSON
    ]
    for bad in refused:
        before = copy.deepcopy(msg)
        if msg.from_json(bad) or msg != before:
            sys.exit("from_json accepted %s" % bad)
        print("refused %s" % bad)


if __name__ == "__main__":
    main()
