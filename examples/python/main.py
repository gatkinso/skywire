"""Fills a Telemetry message, writes it to a file, reads it back and checks
that the copy is equal. See README.md.

    python3 main.py telemetry.bin
"""

import sys

from Position import Position
from Telemetry import Telemetry


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: main.py FILE")
    path = sys.argv[1]

    position = Position()
    position.Set_latitude(47.6062)
    position.Set_longitude(-122.3321)
    position.Set_altitude_m(56.5)

    sent = Telemetry()
    sent.Set_device_id(4207)
    sent.Set_timestamp_us(1760959800123456)
    sent.Set_mode(5)
    sent.Set_status(11)  # status has 3 bits: 11 is stored as 3
    sent.Set_armed(1)
    sent.Set_callsign("SKY1")  # NUL-padded to 8 bytes
    sent.Set_position(position)
    sent.Set_battery_mv(3870)

    # Serialize to Telemetry.SIZE bytes and write them to the file.
    with open(path, "wb") as f:
        f.write(sent.serialize())

    # Read the file back and deserialize a second message.
    with open(path, "rb") as f:
        back = f.read()
    received = Telemetry()
    received.deserialize(back)  # raises ValueError if the input is short

    if received != sent:
        sys.exit("the message read back is different")

    p = received.Get_position()
    print("wrote and read back %d bytes (size() = %d)" % (len(back), received.size()))
    print("device_id %d, callsign %s, status %d, position %.4f, %.4f, %.1f m" % (
        received.Get_device_id(), received.Get_callsign().rstrip(b"\0").decode(),
        received.Get_status(), p.Get_latitude(), p.Get_longitude(), p.Get_altitude_m()))


if __name__ == "__main__":
    main()
