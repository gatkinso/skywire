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

"""Reads a Telemetry from XML written by another program (telemetry.xml: a
declaration, comments, indentation, CDATA, an entity, a string in
hexadecimal and an element the message does not have), writes it back in
the canonical form, and shows what from_xml refuses. See README.md.

    python3 main.py telemetry.xml out.xml
"""

import copy
import sys

from Telemetry import Telemetry


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: main.py INPUT OUTPUT")

    with open(sys.argv[1], "rb") as f:
        text = f.read()

    # The root element must be the message's name; members may come in any
    # order, and an element the message does not have (firmware) is ignored.
    msg = Telemetry()
    if not msg.from_xml(text):
        sys.exit("%s is not the XML form of a Telemetry" % sys.argv[1])
    print("device_id %d, callsign bytes %s, altitude_m %.1f" % (
        msg.Get_device_id(), msg.Get_callsign()[:5].hex(" "),
        msg.Get_position().Get_altitude_m()))

    # to_xml writes the canonical form: no declaration, no whitespace, and
    # the callsign in hexadecimal again, as it holds a byte that is not
    # printable ASCII.
    canonical = msg.to_xml()
    with open(sys.argv[2], "w", encoding="utf-8", newline="") as f:
        f.write(canonical)
    print(canonical)

    # Input that is not a Telemetry, or holds a value a member cannot, is
    # refused, and the message is left as it was.
    refused = [
        '<!DOCTYPE Telemetry [<!ENTITY x "y">]><Telemetry/>',            # no document type declarations
        '<Position/>',                                                   # not a Telemetry
        '<Telemetry><mode>5</Telemetry>',                                # not well-formed
        '<Telemetry><status>8</status></Telemetry>',                     # status has 3 bits
        '<Telemetry><callsign encoding="base64">U0tZMQ==</callsign></Telemetry>',  # hex only
    ]
    for bad in refused:
        before = copy.deepcopy(msg)
        if msg.from_xml(bad) or msg != before:
            sys.exit("from_xml accepted %s" % bad)
        print("refused %s" % bad)


if __name__ == "__main__":
    main()
