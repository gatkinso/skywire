#!/usr/bin/env python3
#
#   Copyright (C) 2026 Geoffrey Atkinson
#
#   Licensed under the Apache License, Version 2.0 (the "License").
#   You may not use this file except in compliance with the License.
#   A copy of the License is located at
#
#   https://www.apache.org/licenses/LICENSE-2.0.html
#
#   or in the "license" file accompanying this file. This file is distributed
#   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#   express or implied. See the License for the specific language governing
#   permissions and limitations under the License.
#
"""Checks the generated C or C++ BSON against pymongo's bson module.

    bson_interop.py <bson_interop or bson_interop_c program> <scratch dir>

Both ways: pymongo decodes what the generated code writes, to the values it
was given, and encodes it again to the same bytes; the generated code reads
what pymongo writes, with the element types pymongo picks and unknown
elements of every type, and rejects values out of range. Prints
PYMONGO_NOT_FOUND (ctest then reports Skipped) without pymongo.
"""

import datetime
import json
import math
import os
import subprocess
import sys

try:
    import bson
    from bson import (Binary, Code, Decimal128, Int64, MaxKey, MinKey,
                      ObjectId, Regex, Timestamp)
except ImportError:
    print("PYMONGO_NOT_FOUND: no bson module (pip install pymongo); skipping")
    sys.exit(0)

failures = []


def check(what, ok):
    if not ok:
        failures.append(what)
        print("FAIL:", what)


def check_written(path, expected):
    """pymongo reads path to expected, and writes the same bytes back."""
    with open(path, "rb") as f:
        data = f.read()
    try:
        doc = bson.decode(data)
    except Exception as e:  # noqa: BLE001 - any decode error is the failure
        check(f"{path}: pymongo cannot decode it: {e}", False)
        return
    check(f"{path}: members in tag order", list(doc) == list(expected))
    for name, want in expected.items():
        got = doc.get(name)
        if isinstance(want, float) and math.isnan(want):
            ok = isinstance(got, float) and math.isnan(got)
        elif isinstance(want, float):
            ok = isinstance(got, float) and got == want and \
                math.copysign(1, got) == math.copysign(1, want)
        elif isinstance(want, int):
            # Int64 or int, by pymongo version; that it is an int64 rather
            # than an int32 is checked by encoding it again, below.
            ok = isinstance(got, int) and not isinstance(got, bool) and got == want
        else:
            ok = type(got) is type(want) and got == want
        check(f"{path}: {name} is {got!r}, not {want!r}", ok)
    check(f"{path}: pymongo writes it back to other bytes",
          bson.encode(doc) == data)


def read(program, scratch, name, doc):
    """The generated code reading pymongo's encoding of doc: its JSON, or
    None if from_bson failed."""
    path = os.path.join(scratch, name + ".bson")
    with open(path, "wb") as f:
        f.write(bson.encode(doc))
    run = subprocess.run([program, "read", path], capture_output=True,
                         text=True, check=False)
    if run.returncode != 0:
        return None
    return json.loads(run.stdout)


def main():
    program, scratch = sys.argv[1], sys.argv[2]
    os.makedirs(scratch, exist_ok=True)
    subprocess.run([program, "write", scratch], check=True)

    # The shared test values. uint32_t, int64_t and uint64_t are int64, which
    # pymongo gives as Int64; the narrower types are int32 (int).
    check_written(os.path.join(scratch, "values.bson"), {
        "a_mydouble100": 3.14159,
        "b_myuint32_200": Int64(3000000000),
        "c_myint64_201": Int64(-9000000000),
        "d_myfloat202": 3.5,
        "e_myint16_203": -12345,
        "f_myuint16_204": 60000,
        "g_myint32_205": -2000000000,
        "h_myuint64_206": Int64(12000000000),
        "i_charcoal301": 65,
        "j_mystring302": "skywire",
        "k_uchar303": 200,
        "l_myuint8_304": 201,
        "m_myint8_305": -50,
        "n_shortstring306": "skyw",
        "o_myfloat1000": 3.5,
        "p_myuint32_2000": Int64(3000000000),
        "q_myint64_2010": Int64(-9000000000),
        "r_mydouble2020": 3.14159,
    })

    # Limits: uint64_t above 2^63-1 as Decimal128, bytes that are not UTF-8
    # as binary, NaN, the infinities and -0.
    check_written(os.path.join(scratch, "extremes.bson"), {
        "a_mydouble100": math.nan,
        "b_myuint32_200": Int64(4294967295),
        "c_myint64_201": Int64(-9223372036854775808),
        "d_myfloat202": -math.inf,
        "e_myint16_203": -32768,
        "f_myuint16_204": 65535,
        "g_myint32_205": -2147483648,
        "h_myuint64_206": Decimal128("18446744073709551615"),
        "i_charcoal301": -128,
        "j_mystring302": b"\x00\xffab",
        "k_uchar303": 255,
        "l_myuint8_304": 0,
        "m_myint8_305": 127,
        "n_shortstring306": "é",
        "o_myfloat1000": 1.401298464324817e-45,
        "p_myuint32_2000": Int64(0),
        "q_myint64_2010": Int64(9223372036854775807),
        "r_mydouble2020": -0.0,
    })

    # pymongo's choice of types, any whole number for an integer, and
    # unknown elements of every type, which are skipped.
    got = read(program, scratch, "pymongo", {
        "_id": ObjectId("0123456789abcdef01234567"),
        "a_mydouble100": 2,
        "b_myuint32_200": Int64(7),
        "c_myint64_201": Decimal128("-90000000000E-1"),
        "d_myfloat202": 1.25,
        "e_myint16_203": -5.0,
        "f_myuint16_204": Decimal128("6553E+1"),
        "h_myuint64_206": Decimal128("1.8446744073709551615E+19"),
        "i_charcoal301": 200,
        "j_mystring302": "héllo",
        "n_shortstring306": Binary(b"\x01\x02", 0),
        "unknown_list": [1, "a", {"b": None}],
        "unknown_doc": {"y": True, "z": [[], {}]},
        "unknown_regex": Regex("a.*", "i"),
        "unknown_date": datetime.datetime(2026, 11, 9, tzinfo=datetime.timezone.utc),
        "unknown_timestamp": Timestamp(1, 2),
        "unknown_min": MinKey(),
        "unknown_max": MaxKey(),
        "unknown_code": Code("f()", {"s": 1}),
        "unknown_plain_code": Code("g()"),
        "unknown_uuid": Binary(b"0123456789abcdef", 4),
        "unknown_nan": Decimal128("NaN"),
        "q_myint64_2010": Int64(3),
    })
    check("reading pymongo's document failed", got is not None)
    if got is not None:
        want = {
            "a_mydouble100": 2, "b_myuint32_200": 7,
            "c_myint64_201": "-9000000000", "d_myfloat202": 1.25,
            "e_myint16_203": -5, "f_myuint16_204": 65530,
            "g_myint32_205": 0, "h_myuint64_206": "18446744073709551615",
            "i_charcoal301": -56,
            # Each byte of the UTF-8 is one character in the JSON.
            "j_mystring302": "héllo".encode().decode("latin-1"),
            "k_uchar303": 0, "n_shortstring306": "\x01\x02",
            "q_myint64_2010": "3",
        }
        for name, value in want.items():
            check(f"read {name} as {got.get(name)!r}, not {value!r}",
                  got.get(name) == value)

    # Each of these must fail.
    for name, doc in [
        ("negative_uint64", {"h_myuint64_206": -1}),
        ("uint64_too_big", {"h_myuint64_206": Decimal128("18446744073709551616")}),
        ("int16_too_big", {"e_myint16_203": 40000}),
        ("not_whole", {"b_myuint32_200": 1.5}),
        ("decimal_not_whole", {"c_myint64_201": Decimal128("1.5")}),
        ("decimal_infinity", {"c_myint64_201": Decimal128("Infinity")}),
        ("string_too_long", {"n_shortstring306": "abcde"}),
        ("wrong_type", {"g_myint32_205": "1"}),
        ("uuid_string", {"j_mystring302": Binary(b"0123456789abcdef", 4)}),
    ]:
        check(f"{name} was read", read(program, scratch, name, doc) is None)

    if failures:
        print(f"{len(failures)} failure(s)")
        return 1
    print("pymongo", bson.__name__, "agrees with the generated BSON")
    return 0


if __name__ == "__main__":
    sys.exit(main())
