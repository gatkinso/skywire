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
"""Run skywire's libFuzzer targets for a while each.

    run_fuzzers.py BUILD                     # every target, 60 s each
    run_fuzzers.py BUILD --seconds 600 fuzz_spec fuzz_lang

BUILD is a build directory configured with -DSKYWIRE_FUZZ=ON and built with
--target fuzzers (src/test/fuzz/README.md). Each target starts from its
seeds, listed in BUILD/fuzz/<target>.fuzz, plus whatever its working corpus
(WORK/<target>) kept from earlier runs. Targets run in parallel, one per CPU
by default.

A target that crashes, times out, runs out of memory or fails a check
leaves its input in ARTIFACTS/<target>/. The script prints how to reproduce
it, and exits 1.
"""

import argparse
import concurrent.futures
import glob
import os
import re
import shutil
import subprocess
import sys


def read_manifest(path):
    target = {"name": os.path.basename(path)[:-len(".fuzz")], "seeds": []}
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            key, _, value = line.partition("=")
            if key == "seed":
                target["seeds"].append(value)
            elif key:
                target[key] = value
    return target


def seed_corpus(target, corpus):
    """Copies the target's seeds into its working corpus."""
    os.makedirs(corpus, exist_ok=True)
    for seed in target["seeds"]:
        files = ([os.path.join(seed, name) for name in sorted(os.listdir(seed))]
                 if os.path.isdir(seed) else [seed])
        for path in files:
            if os.path.isfile(path):
                shutil.copy(path, os.path.join(corpus, os.path.basename(path)))


def run(target, args):
    corpus = os.path.join(args.work, target["name"])
    artifacts = os.path.join(args.artifacts, target["name"])
    seed_corpus(target, corpus)
    os.makedirs(artifacts, exist_ok=True)

    command = [target["program"],
               f"-max_total_time={args.seconds}",
               "-timeout=10",
               "-rss_limit_mb=2048",
               "-print_final_stats=1",
               f"-artifact_prefix={artifacts}{os.sep}"]
    if "dict" in target:
        command.append(f"-dict={target['dict']}")
    command.append(corpus)

    log = os.path.join(args.work, target["name"] + ".log")
    with open(log, "w", encoding="utf-8") as out:
        result = subprocess.run(command, stdout=out, stderr=subprocess.STDOUT)
    with open(log, encoding="utf-8", errors="replace") as f:
        text = f.read()

    stats = {}
    for key in ("number_of_executed_units", "average_exec_per_sec"):
        match = re.search(rf"stat::{key}:\s*(\d+)", text)
        stats[key] = match.group(1) if match else "?"
    covs = re.findall(r"cov: (\d+)", text)
    return {
        "target": target,
        "returncode": result.returncode,
        "runs": stats["number_of_executed_units"],
        "per_sec": stats["average_exec_per_sec"],
        "cov": covs[-1] if covs else "?",
        "log": log,
        "text": text,
        "artifacts": sorted(glob.glob(os.path.join(artifacts, "*"))),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("build", help="build directory (-DSKYWIRE_FUZZ=ON)")
    parser.add_argument("targets", nargs="*", help="targets to run (default: all)")
    parser.add_argument("--seconds", type=int, default=60, help="time per target (default 60)")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 1,
                        help="targets run at once (default: one per CPU)")
    parser.add_argument("--work", default=None, help="working corpora and logs (default BUILD/fuzz/work)")
    parser.add_argument("--artifacts", default=None, help="failing inputs (default BUILD/fuzz/artifacts)")
    args = parser.parse_args()
    args.work = os.path.abspath(args.work or os.path.join(args.build, "fuzz", "work"))
    args.artifacts = os.path.abspath(args.artifacts or os.path.join(args.build, "fuzz", "artifacts"))

    manifests = sorted(glob.glob(os.path.join(args.build, "fuzz", "*.fuzz")))
    targets = [read_manifest(path) for path in manifests]
    if not targets:
        sys.exit(f"run_fuzzers.py: no fuzz targets in {args.build}; configure it with -DSKYWIRE_FUZZ=ON")
    if args.targets:
        unknown = set(args.targets) - {t["name"] for t in targets}
        if unknown:
            sys.exit(f"run_fuzzers.py: unknown targets: {', '.join(sorted(unknown))}")
        targets = [t for t in targets if t["name"] in args.targets]
    for target in targets:
        if target.get("fuzzing") != "ON":
            sys.exit(f"run_fuzzers.py: {args.build} is not a -DSKYWIRE_FUZZ=ON build")
        if not os.path.isfile(target["program"]):
            sys.exit(f"run_fuzzers.py: {target['program']} is not built; "
                     f"run cmake --build {args.build} --target fuzzers")

    print(f"Fuzzing {len(targets)} targets for {args.seconds} s each, {args.jobs} at a time", flush=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(lambda t: run(t, args), targets))

    print(f"\n{'target':<18}{'result':<8}{'runs':>12}{'exec/s':>10}{'cov':>8}")
    for r in results:
        status = "ok" if r["returncode"] == 0 and not r["artifacts"] else "FAILED"
        print(f"{r['target']['name']:<18}{status:<8}{r['runs']:>12}{r['per_sec']:>10}{r['cov']:>8}")

    failed = [r for r in results if r["returncode"] != 0 or r["artifacts"]]
    for r in failed:
        name = r["target"]["name"]
        print(f"\n=== {name} failed (exit {r['returncode']}); log: {r['log']}")
        lines = r["text"].splitlines()
        start = next((i for i, line in enumerate(lines)
                      if "ERROR:" in line or "FUZZ_CHECK failed" in line
                      or "runtime error" in line or "terminating" in line), max(0, len(lines) - 40))
        print("\n".join(lines[start:start + 40]))
        for artifact in r["artifacts"]:
            print(f"\nReproduce: {r['target']['program']} {artifact}")
        print(f"Once fixed, add the input to src/test/fuzz/corpus/{name}/ so ctest replays it.")

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
