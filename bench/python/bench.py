"""Times the generated Python code for the ring message. See ../README.md.

    python3 bench.py RingyMcRingface.bin
"""

import sys
import timeit

from RingyMcRingface import RingyMcRingface

ROUNDS = 5
ITERS = 100_000


def best_ns(body):
    """Best time per call, in nanoseconds, over ROUNDS rounds of ITERS calls."""
    return min(timeit.repeat(body, number=ITERS, repeat=ROUNDS)) / ITERS * 1e9


def main():
    if len(sys.argv) != 2:
        sys.exit("usage: python3 bench.py RingyMcRingface.bin")
    with open(sys.argv[1], "rb") as f:
        data = f.read()

    # The message must decode and re-encode to the same bytes.
    m = RingyMcRingface()
    m.deserialize(data)
    if m.serialize() != data:
        sys.exit("serialize does not reproduce the input")

    m2 = RingyMcRingface()
    ser = best_ns(m.serialize)
    de = best_ns(lambda: m2.deserialize(data))

    print("RESULT serialize %.1f" % ser)
    print("RESULT deserialize %.1f" % de)
    print("NOTE Python %s" % sys.version.split()[0])


if __name__ == "__main__":
    main()
