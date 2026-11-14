#!/usr/bin/env python3
"""Check the JSON Schemas in schema/ against the specs and lang files in the repo.

    check_schemas.py [--wiregen PATH]

Every spec in examples/specs and the generated tests, and every lang file in
lang/, must validate. A list of broken specs, one per check wiregen makes, must
each fail validation, and with --wiregen, wiregen must reject each of them too
and accept a spec that names its schema with "$schema".

Needs the jsonschema module. Without it this prints JSONSCHEMA_NOT_FOUND (the
ctest's skip marker) and exits 0.
"""

import argparse
import copy
import glob
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPEC_SCHEMA = os.path.join(ROOT, "schema", "skywire-spec.schema.json")
LANG_SCHEMA = os.path.join(ROOT, "schema", "skywire-lang.schema.json")
SPEC_GLOBS = [
    "examples/specs/*.json",
    "src/test/generated_tests/*.json",
    "src/test/generated_tests/ring/*.json",
    "packaging/conan/conan-center/all/test_package/*.json",
]
LANG_GLOBS = ["lang/*/lang.*.json"]

GOOD = {
    "stringValues": {"classname": "Point", "namespace": "geo::shapes", "bitorder": "msb"},
    "members": {
        "1": {"type": "uint16_t", "name": "x", "endian": 1, "bits": 12},
        "2": {"type": "int16_t", "name": "y", "bits": 4},
        "3": {"type": "string", "name": "label", "length": 8},
        "536870911": {"type": "double", "name": "z", "endian": 2},
    },
}


def broken(change):
    """GOOD with change(spec) applied."""
    spec = copy.deepcopy(GOOD)
    change(spec)
    return spec


def member(tag, **fields):
    return lambda spec: spec["members"].__setitem__(tag, fields)


# Each breaks one rule wiregen checks (Processor::validate_spec, or the JSON
# parse for unknown fields).
BAD = {
    "no classname": broken(lambda s: s["stringValues"].pop("classname")),
    "classname not an identifier": broken(lambda s: s["stringValues"].update(classname="2D")),
    "namespace with a slash": broken(lambda s: s["stringValues"].update(namespace="geo/shapes")),
    "bitorder neither lsb nor msb": broken(lambda s: s["stringValues"].update(bitorder="big")),
    "output file in a directory": broken(lambda s: s["stringValues"].update(outputimplfilename="sub/Point.c")),
    "include with a space": broken(lambda s: s.update(includes=["<my header.h>"])),
    "no members": broken(lambda s: s.update(members={})),
    "tag 0": broken(member("0", type="uint8_t", name="t")),
    "tag over 2^29-1": broken(member("536870912", type="uint8_t", name="t")),
    "tag reserved by protobuf": broken(member("19500", type="uint8_t", name="t")),
    "name not an identifier": broken(member("4", type="uint8_t", name="has space")),
    "type not an identifier": broken(member("4", type="Point*", name="t")),
    "string without a length": broken(member("4", type="string", name="t")),
    "string of length 0": broken(member("4", type="string", name="t", length=0)),
    "bits on a float": broken(member("4", type="float", name="t", bits=3)),
    "bits wider than the type": broken(member("4", type="uint8_t", name="t", bits=9)),
    "endian 3": broken(member("4", type="uint32_t", name="t", endian=3)),
    "unknown member field": broken(member("4", type="uint32_t", name="t", size=4)),
    "unknown top-level field": broken(lambda s: s.update(member={})),
}


def load(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def files(globs):
    found = []
    for pattern in globs:
        found += sorted(glob.glob(os.path.join(ROOT, pattern)))
    return found


def check_files(validator, paths, what):
    failures = 0
    for path in paths:
        errors = list(validator.iter_errors(load(path)))
        for error in errors:
            where = "/".join(str(p) for p in error.absolute_path) or "(top)"
            print(f"FAIL {os.path.relpath(path, ROOT)}: {where}: {error.message}")
        failures += bool(errors)
    print(f"{len(paths) - failures}/{len(paths)} {what} valid")
    return failures


def wiregen_accepts(wiregen, spec, outdir):
    path = os.path.join(outdir, "spec.json")
    with open(path, "w", encoding="utf-8") as f:
        json.dump(spec, f)
    result = subprocess.run(
        [wiregen, "--language", "c++", "--input_file", path, "--outdir", outdir,
         "--write_to_file", "--silent"],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return result.returncode == 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--wiregen", help="also check wiregen agrees on the broken specs")
    args = parser.parse_args()

    try:
        import jsonschema
    except ImportError:
        print("JSONSCHEMA_NOT_FOUND: the jsonschema Python module is not installed; skipping")
        return 0

    validators = {}
    for path in (SPEC_SCHEMA, LANG_SCHEMA):
        schema = load(path)
        jsonschema.Draft202012Validator.check_schema(schema)
        validators[path] = jsonschema.Draft202012Validator(schema)
    spec_validator = validators[SPEC_SCHEMA]

    failures = check_files(spec_validator, files(SPEC_GLOBS), "specs")
    failures += check_files(validators[LANG_SCHEMA], files(LANG_GLOBS), "lang files")

    named = dict(GOOD, **{"$schema": "skywire-spec.schema.json"})
    for name, spec in (("good spec", GOOD), ("good spec with $schema", named)):
        if not spec_validator.is_valid(spec):
            print(f"FAIL {name}: the schema rejects it")
            failures += 1

    for name, spec in BAD.items():
        if spec_validator.is_valid(spec):
            print(f"FAIL {name}: the schema accepts it")
            failures += 1
    print(f"{len(BAD)} broken specs checked against the schema")

    if args.wiregen:
        with tempfile.TemporaryDirectory() as outdir:
            for name, spec in (("good spec", GOOD), ("good spec with $schema", named)):
                if not wiregen_accepts(args.wiregen, spec, outdir):
                    print(f"FAIL {name}: wiregen rejects it")
                    failures += 1
            for name, spec in BAD.items():
                if wiregen_accepts(args.wiregen, spec, outdir):
                    print(f"FAIL {name}: wiregen accepts it")
                    failures += 1
        print(f"{len(BAD)} broken specs checked against wiregen")

    if failures:
        print(f"{failures} failure(s)")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
