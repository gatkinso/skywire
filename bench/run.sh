#!/usr/bin/env bash
#
#  Copyright (C) 2026 Skyloupe, LLC
#
#  Licensed under the Apache License, Version 2.0 (the "License").
#  You may not use this file except in compliance with the License.
#  A copy of the License is located at
#
#  https://www.apache.org/licenses/LICENSE-2.0.html
#
#  or in the "license" file accompanying this file. This file is distributed
#  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#  express or implied. See the License for the specific language governing
#  permissions and limitations under the License.
#
#  Times the generated code of every language on the ring message and prints
#  one table. See README.md.
#
#    bench/run.sh --build <build-dir> [lang ...]
#    bench/run.sh --wiregen <path> [--c <bench_c>] [--cpp <bench_cpp>] [--workdir <dir>] [lang ...]
#
#  Languages: c cpp swift go python java rust (default: all). The C and C++
#  benchmarks are built by CMake with -DSKYWIRE_BENCH=ON; the others are
#  generated and compiled here. A language whose toolchain is missing is
#  reported as skipped.

set -u

here=$(cd "$(dirname "$0")" && pwd)
root=$(dirname "$here")
ring=$root/src/test/generated_tests/ring
input=$ring/RingyMcRingface.bin

all_langs="c cpp swift go python java rust"
wiregen="" bench_c="" bench_cpp="" workdir="" langs=""

exe=""
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) exe=".exe" ;;
esac

usage() {
    sed -n '/^#    bench/p' "$0" | sed 's/^#    /usage: /' >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --build)
            [ $# -ge 2 ] || usage
            wiregen=$2/src/bin/wiregen/wiregen$exe
            bench_c=$2/bench/bench_c$exe
            bench_cpp=$2/bench/bench_cpp$exe
            workdir=$2/bench/run
            shift 2 ;;
        --wiregen) [ $# -ge 2 ] || usage; wiregen=$2; shift 2 ;;
        --c)       [ $# -ge 2 ] || usage; bench_c=$2; shift 2 ;;
        --cpp)     [ $# -ge 2 ] || usage; bench_cpp=$2; shift 2 ;;
        --workdir) [ $# -ge 2 ] || usage; workdir=$2; shift 2 ;;
        -h|--help) usage ;;
        -*) echo "unknown option $1" >&2; usage ;;
        *)
            case " $all_langs " in
                *" $1 "*) langs="$langs $1" ;;
                *) echo "unknown language '$1' (use: $all_langs)" >&2; exit 2 ;;
            esac
            shift ;;
    esac
done

[ -n "$wiregen" ] || usage
[ -x "$wiregen" ] || { echo "wiregen not found at $wiregen" >&2; exit 2; }
[ -n "$workdir" ] || workdir=$(pwd)/bench_run
[ -n "$langs" ] || langs=$all_langs

# rustup installs into ~/.cargo/bin, which a non-interactive shell may lack.
PATH=$PATH:$HOME/.cargo/bin
export GOTOOLCHAIN=local

rm -rf "$workdir"
mkdir -p "$workdir"

# Generates one language's ring code into $workdir/<lang>; extra arguments go
# to wiregen.
generate() {
    local lang=$1
    shift
    local dir=$workdir/$lang
    mkdir -p "$dir"
    "$wiregen" --input_file "$ring/RingyInner.json" --outdir "$dir" \
               --language "$lang" --write_to_file "$@" &&
    "$wiregen" --input_file "$ring/RingyMcRingface.json" --outdir "$dir" \
               --language "$lang" --write_to_file "$@"
}

have() {
    command -v "$1" >/dev/null 2>&1
}

# Each bench_<lang> function builds (if needed) and runs one benchmark, which
# prints "RESULT <metric> <ns>" and "NOTE <text>" lines. It returns 3 when the
# toolchain is missing.

bench_c() {
    [ -n "$bench_c" ] && [ -x "$bench_c" ] || { skip_reason="bench_c not built (configure with -DSKYWIRE_BENCH=ON)"; return 3; }
    "$bench_c" "$input"
}

bench_cpp() {
    [ -n "$bench_cpp" ] && [ -x "$bench_cpp" ] || { skip_reason="bench_cpp not built (configure with -DSKYWIRE_BENCH=ON)"; return 3; }
    "$bench_cpp" "$input"
}

bench_swift() {
    have swiftc || { skip_reason="swiftc not on PATH"; return 3; }
    local dir=$workdir/swift
    generate swift || return 1
    cp "$here/swift/main.swift" "$dir/"
    swiftc -O -wmo -o "$dir/bench_swift$exe" "$root/lang/swift/SWStream.swift" \
        "$dir/RingyInner.swift" "$dir/RingyMcRingface.swift" "$dir/main.swift" || return 1
    "$dir/bench_swift$exe" "$input"
}

bench_go() {
    have go || { skip_reason="go not on PATH"; return 3; }
    local dir=$workdir/go
    # The benchmark is a command, so the generated code is in package main.
    generate go --namespace main || return 1
    cp "$here/go/bench.go" "$dir/"
    printf 'module wiregen/bench\n\ngo 1.21\n' > "$dir/go.mod"
    (cd "$dir" && go build -o "bench_go$exe" .) || return 1
    "$dir/bench_go$exe" "$input"
}

bench_python() {
    # Windows may have a python3 that only opens the Microsoft Store.
    local py=""
    for candidate in python3 python; do
        if "$candidate" -c 'import sys; sys.exit(sys.version_info[0] != 3)' >/dev/null 2>&1; then
            py=$candidate
            break
        fi
    done
    [ -n "$py" ] || { skip_reason="python3 not on PATH"; return 3; }
    local dir=$workdir/python
    generate python || return 1
    cp "$here/python/bench.py" "$dir/"
    "$py" "$dir/bench.py" "$input"
}

bench_java() {
    have javac && have java && javac -version >/dev/null 2>&1 || { skip_reason="no JDK on PATH"; return 3; }
    local dir=$workdir/java
    generate java || return 1
    cp "$here/java/Bench.java" "$dir/"
    javac -d "$dir/classes" "$dir/RingyInner.java" "$dir/RingyMcRingface.java" "$dir/Bench.java" || return 1
    java -cp "$dir/classes" Bench "$input"
}

bench_rust() {
    have rustc || { skip_reason="rustc not on PATH or in ~/.cargo/bin"; return 3; }
    local dir=$workdir/rust
    generate rust || return 1
    cp "$here/rust/main.rs" "$dir/"
    # Under Git Bash on Windows, /usr/bin/link (coreutils) would shadow the
    # MSVC link.exe that rustc needs, so leave it off rustc's PATH.
    local rust_path=$PATH
    if [ -n "$exe" ]; then
        rust_path=$(printf '%s' "$PATH" | awk -v RS=: -v ORS=: '$0 != "/usr/bin" && $0 != "/bin"')
    fi
    PATH=$rust_path rustc --edition 2021 -O -o "$dir/bench_rust$exe" "$dir/main.rs" || return 1
    "$dir/bench_rust$exe" "$input"
}

display() {
    case "$1" in
        c) echo "C" ;; cpp) echo "C++" ;; swift) echo "Swift" ;; go) echo "Go" ;;
        python) echo "Python" ;; java) echo "Java" ;; rust) echo "Rust" ;;
    esac
}

rows=""
failed=0
for lang in $langs; do
    name=$(display "$lang")
    echo "running $name..." >&2
    skip_reason=""
    log=$workdir/$lang.log
    mkdir -p "$workdir"
    "bench_$lang" > "$log" 2>&1
    rc=$?
    if [ $rc -eq 3 ]; then
        rows="$rows| $name | | | skipped: $skip_reason |"$'\n'
        continue
    fi
    ser=$(awk '$1 == "RESULT" && $2 == "serialize" { print $3 }' "$log")
    de=$(awk '$1 == "RESULT" && $2 == "deserialize" { print $3 }' "$log")
    notes=$(awk '$1 == "NOTE" { sub(/^NOTE /, ""); out = out (n++ ? "; " : "") $0 } END { print out }' "$log")
    if [ $rc -ne 0 ] || [ -z "$ser" ] || [ -z "$de" ]; then
        echo "$name failed (exit $rc):" >&2
        cat "$log" >&2
        rows="$rows| $name | | | FAILED (see $log) |"$'\n'
        failed=1
        continue
    fi
    rows="$rows| $name | $ser | $de | $notes |"$'\n'
done

echo
echo "Ring message ($(wc -c < "$input" | tr -d ' ') bytes), best of several rounds, ns per message"
echo
echo "| Language | Serialize | Deserialize | Notes |"
echo "|---|---:|---:|---|"
printf '%s' "$rows"
exit $failed
