# Benchmarks

Times the code wiregen generates for each language on one message: the ring
message from the cross-language serial test
(`src/test/generated_tests/ring`, 135 bytes, mixed byte order on nearly every
field, a nested message, bitfields and fixed-length strings). Every benchmark
first checks that the message decodes from `RingyMcRingface.bin` and encodes
back to the same bytes.

The benchmarks are not part of ctest.

## Running

Use a Release build directory without sanitizers:

```
cmake -S skywire -B build-bench -DCMAKE_BUILD_TYPE=Release -DSKYWIRE_BENCH=ON <your usual -D options>
cmake --build build-bench --target bench
```

The `bench` target builds wiregen and the C and C++ benchmarks, then runs
`run.sh`, which generates, compiles and runs the Swift, Go, Python, Java, Rust,
C# and JavaScript benchmarks and prints one table:

```
| Language | Serialize | Deserialize | Notes |
|---|---:|---:|---|
| C | 7.9 | 10.3 |  |
| C++ | 7.8 | 11.9 | reused-stream round trip 14.3 ns |
...
```

[docs/testing.md](../docs/testing.md#benchmarks) has a full table from one machine.

To run some languages only, or again without rebuilding:

```
bench/run.sh --build build-bench                 # every language
bench/run.sh --build build-bench swift python    # only these
```

The languages are `c cpp swift go python java rust csharp javascript`. A language whose
toolchain is missing is listed as skipped. A full run takes about a minute.

## What is timed

Times are nanoseconds per message, the best of several rounds of many
iterations each (Java, C# and JavaScript run extra rounds to warm up the JIT). Expect ±10%
between runs; compare numbers from the same machine.

| Language | Serialize | Deserialize |
|---|---|---|
| C | `RingyMcRingface_Serialize` into a reused `Cstream`, emptied each time | copy the 135 bytes into the `Cstream`, then `RingyMcRingface_Deserialize` |
| C++ | `serialize` into a reused `skyloupe::Stream` (SW mode), then `clear()` | `write` the 135 bytes into the stream, then `deserialize` |
| Swift | `serialize` into a reused `SWStream`, then `clear()` | `write` the 135 bytes into the stream, then `deserialize` |
| Go | `AppendTo` a reused slice | `Deserialize(bin)` |
| Python | `serialize()` (returns new `bytes`) | `deserialize(bin)` |
| Java | `serialize(ByteBuffer)` into a reused buffer | `deserialize(byte[])` |
| Rust | `serialize_into` a reused `Vec` | `deserialize(&bin)` |
| C# | `Serialize(buf, ref offset)` into a reused `byte[]` | `Deserialize(bin)` |
| JavaScript | `serializeInto` a reused `DataView` | `deserializeFrom(bin)`, a `Uint8Array` |

Notes give the other forms:

- **C++, Swift:** a serialize then deserialize round trip through one reused
  stream.
- **Swift:** how much the process's peak memory grew over 200,000 round trips
  through one reused stream. A stream that keeps the bytes it has already
  read grows here. Not measured on Windows.
- **Go:** `Serialize()` to a new slice, and allocations per operation.
- **Java:** `serialize()` to a new `byte[]`, and `hashCode()`.
- **Rust:** `serialize()` to a new `Vec`.
- **C#:** `Serialize()` to a new `byte[]`, `GetHashCode()`, and the .NET
  version.
- **JavaScript:** `serialize()` to a new `Uint8Array`, and the Node version.

The C and C++ benchmarks use the build's compiler flags; the others are built
with `swiftc -O -wmo` (whole-module, as SwiftPM and Xcode release builds do), `rustc -O`, `go build` and `javac` defaults, and C# with `dotnet build -c Release`
for the framework the installed SDK ships with. JavaScript runs under `node`
on `PATH` with its defaults.

## Windows

Configure with Ninja and `-DCMAKE_BUILD_TYPE=Release` from a VS developer
prompt (vcvars64), the same as a normal build with `-DSKYWIRE_BENCH=ON`
added. The `bench` target runs `run.sh` with Git for Windows' bash, not the
`bash.exe` in System32 (which is WSL). From Git Bash, run
`bench/run.sh --build C:/path/to/build-bench` the same as elsewhere, from a
shell that has the VS environment so rustc finds the MSVC linker.

Python is whichever of `python3` and `python` on `PATH` runs first; the
Microsoft Store placeholder is skipped. Put the Python you want first on `PATH`.

Without bash, run `bench_c.exe` and `bench_cpp.exe` from the build's `bench`
directory directly, with the path of `RingyMcRingface.bin` as their argument.

## wiregen itself

`wiregen_scale.py` times wiregen rather than the code it generates: how long
generating takes as messages grow. It writes synthetic specifications (flat
messages of 100 to 4000 members cycling through every native type, strings
and bitfields, and chains of 2 to 64 nested messages) and prints the seconds
per wiregen run for each language, generating tests and relays too. It needs
only Python 3 and a wiregen, built or installed:

```
bench/wiregen_scale.py                                  # wiregen on PATH
bench/wiregen_scale.py --wiregen build/bin/wiregen
bench/wiregen_scale.py --sizes 1000 10000 --languages c c++ --runs 5
```

`--keep DIR` keeps the specifications and generated files. Times include
starting wiregen and reading its lang file, a few milliseconds. On an Apple
M1 Pro (October 2026):

| Language | 100 members | 1000 members | 4000 members | Output at 4000 |
|---|---:|---:|---:|---:|
| C | 0.008 | 0.032 | 0.093 | 3.8 MB |
| C++ | 0.008 | 0.038 | 0.119 | 4.1 MB |
| C# | 0.017 | 0.123 | 0.520 | 2.9 MB |
| Go | 0.012 | 0.055 | 0.200 | 1.9 MB |
| Java | 0.016 | 0.087 | 0.364 | 2.9 MB |
| JavaScript | 0.016 | 0.117 | 0.426 | 2.6 MB |
| proto3 | 0.006 | 0.010 | 0.028 | 0.3 MB |
| Python | 0.009 | 0.025 | 0.092 | 1.6 MB |
| Rust | 0.012 | 0.068 | 0.233 | 2.0 MB |
| Swift | 0.007 | 0.018 | 0.068 | 2.4 MB |

Time grows linearly with the number of members (checked to 8000 on macOS,
Linux and Windows). A chain of 64 nested messages takes under 10 ms in every
language. Expect a few times longer on older or virtual machines: a 2017
Intel i7-7700 took 0.3 to 1.0 s at 4000 members, and a 3-vCPU Windows VM
0.1 to 4 s, with C#, Java and JavaScript the slowest there.

## Files

| File | |
|---|---|
| `CMakeLists.txt` | generates the C and C++ ring code and builds `bench_c` and `bench_cpp`; adds the `bench` target |
| `run.sh` | generates and builds the other languages, runs everything, prints the table |
| `wiregen_scale.py` | times wiregen itself on large and deeply nested messages |
| `bench_c.c`, `bench_cpp.cpp` | C and C++ benchmarks |
| `swift/main.swift`, `go/bench.go`, `python/bench.py`, `java/Bench.java`, `rust/main.rs`, `csharp/Bench.cs`, `csharp/Bench.csproj` and `javascript/bench.js` | the other languages, copied next to their generated code in the work directory (`<build>/bench/run`) |

Each benchmark prints `RESULT serialize <ns>`, `RESULT deserialize <ns>` and
any `NOTE <text>` lines; `run.sh` collects them into the table.
