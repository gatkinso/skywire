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
"""Time wiregen itself on large and deeply nested messages.

    wiregen_scale.py                              # wiregen on PATH
    wiregen_scale.py --wiregen build/bin/wiregen
    wiregen_scale.py --sizes 1000 10000 --languages c cpp

Generates synthetic specifications and prints two tables of seconds per
wiregen run (the best of --runs), with tests and relays generated as well:

- flat messages with --sizes members each, cycling through every native type,
  fixed-length strings and bitfield pairs;
- chains of --depths nested messages, each holding the next.

The time includes starting wiregen and reading its lang file, so the smallest
specs mostly measure that. Generated files go to a temporary directory
(--keep keeps them).
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time

LANGUAGES = ["c", "c++", "csharp", "go", "java", "javascript", "proto3", "python3", "rust", "swift"]
CYCLE = ["uint8_t", "int16_t", "uint32_t", "int64_t", "float", "double", "string", "bits"]


def flat_spec(members):
    """A message of the given number of members, cycling through CYCLE."""
    spec = {}
    tag = 1
    i = 0
    while len(spec) < members:
        kind = CYCLE[i % len(CYCLE)]
        i += 1
        if kind == "string":
            spec[str(tag)] = {"type": "string", "name": f"m{tag}", "length": 8}
        elif kind == "bits":
            spec[str(tag)] = {"type": "uint8_t", "name": f"m{tag}", "bits": 4}
            if len(spec) < members:
                tag += 1
                spec[str(tag)] = {"type": "uint8_t", "name": f"m{tag}", "bits": 4}
        else:
            spec[str(tag)] = {"type": kind, "name": f"m{tag}", "endian": tag % 2}
        tag += 1
    return {"stringValues": {"classname": "Big", "namespace": "scale"}, "members": spec}


def write_chain(depth, directory):
    """Writes N0.json .. N<depth-1>.json, each holding the next; returns N0's path."""
    os.makedirs(directory, exist_ok=True)
    for k in range(depth):
        members = {"1": {"type": "uint32_t", "name": "a", "endian": 0},
                   "2": {"type": "double", "name": "b", "endian": 1}}
        if k + 1 < depth:
            members["3"] = {"type": f"N{k + 1}", "name": "next"}
        with open(os.path.join(directory, f"N{k}.json"), "w", encoding="utf-8") as f:
            json.dump({"stringValues": {"classname": f"N{k}", "namespace": "scale"}, "members": members}, f)
    return os.path.join(directory, "N0.json")


def time_wiregen(wiregen, spec, language, outdir, runs):
    """Best time of runs; returns (seconds, bytes written) or (None, first error line)."""
    best = None
    for _ in range(runs):
        shutil.rmtree(outdir, ignore_errors=True)
        os.makedirs(outdir)
        start = time.perf_counter()
        result = subprocess.run([wiregen, "--language", language, "--input_file", spec,
                                 "--generate_tests", "--generate_relay", "--write_to_file", "--silent",
                                 "--outdir", outdir], capture_output=True, text=True)
        elapsed = time.perf_counter() - start
        if result.returncode != 0:
            lines = (result.stderr or result.stdout).strip().splitlines()
            return None, lines[0] if lines else f"exit {result.returncode}"
        best = elapsed if best is None else min(best, elapsed)
    size = sum(os.path.getsize(os.path.join(outdir, name)) for name in os.listdir(outdir))
    return best, size


def megabytes(size):
    if size is None:
        return ""
    return f"{size / 1e6:.1f} MB" if size >= 1e6 else f"{size / 1e3:.0f} KB"


def table(title, column, values, languages, cell, extra=None):
    print(f"\n{title}\n")
    header = f"| Language | " + " | ".join(f"{v} {column}" for v in values) + " |"
    if extra:
        header += f" {extra[0]} |"
    print(header)
    print("|---|" + "---:|" * (len(values) + (1 if extra else 0)))
    errors = []
    for language in languages:
        cells = []
        for value in values:
            seconds, detail = cell(language, value)
            if seconds is None:
                cells.append("failed")
                errors.append(f"{language}, {value} {column}: {detail}")
            else:
                cells.append(f"{seconds:.3f}")
        row = f"| {language} | " + " | ".join(cells) + " |"
        if extra:
            row += f" {extra[1](language)} |"
        print(row, flush=True)
    for error in errors:
        print(f"  {error}")
    return not errors


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--wiregen", default=shutil.which("wiregen"), help="wiregen to time (default: the one on PATH)")
    parser.add_argument("--languages", nargs="+", default=LANGUAGES, metavar="LANG", help="wiregen --language names")
    parser.add_argument("--sizes", nargs="+", type=int, default=[100, 500, 1000, 2000, 4000], metavar="N",
                        help="members per flat message")
    parser.add_argument("--depths", nargs="+", type=int, default=[2, 8, 32, 64], metavar="D",
                        help="messages per nested chain")
    parser.add_argument("--runs", type=int, default=3, help="runs per cell; the best is shown (default 3)")
    parser.add_argument("--keep", metavar="DIR", help="write specs and generated files to DIR and keep them")
    args = parser.parse_args()

    if not args.wiregen or not os.path.isfile(args.wiregen):
        sys.exit("wiregen_scale.py: no wiregen found; give --wiregen or put it on PATH")
    if args.keep:
        os.makedirs(args.keep, exist_ok=True)
    work = args.keep or tempfile.mkdtemp(prefix="wiregen_scale_")
    outdir = os.path.join(work, "out")

    print(f"wiregen: {args.wiregen}")
    print("Seconds per run (best of {}), generating code, tests and relay.".format(args.runs))

    flat = {}
    for n in args.sizes:
        flat[n] = os.path.join(work, f"Big{n}.json")
        with open(flat[n], "w", encoding="utf-8") as f:
            json.dump(flat_spec(n), f)
    largest = max(args.sizes)
    sizes = {}

    def flat_cell(language, n):
        seconds, detail = time_wiregen(args.wiregen, flat[n], language, outdir, args.runs)
        if seconds is not None and n == largest:
            sizes[language] = detail
        return seconds, detail

    ok = table("Flat messages", "members", args.sizes, args.languages, flat_cell,
               (f"Output at {largest}", lambda language: megabytes(sizes.get(language))))

    chains = {d: write_chain(d, os.path.join(work, f"chain{d}")) for d in args.depths}
    ok = table("Nested chains (the outermost message)", "deep", args.depths, args.languages,
               lambda language, d: time_wiregen(args.wiregen, chains[d], language, outdir, args.runs)) and ok

    if not args.keep:
        shutil.rmtree(work, ignore_errors=True)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
