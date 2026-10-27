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

The main README (section 4.1) has a full table from one machine.

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

## Files

| File | |
|---|---|
| `CMakeLists.txt` | generates the C and C++ ring code and builds `bench_c` and `bench_cpp`; adds the `bench` target |
| `run.sh` | generates and builds the other languages, runs everything, prints the table |
| `bench_c.c`, `bench_cpp.cpp` | C and C++ benchmarks |
| `swift/main.swift`, `go/bench.go`, `python/bench.py`, `java/Bench.java`, `rust/main.rs`, `csharp/Bench.cs`, `csharp/Bench.csproj` and `javascript/bench.js` | the other languages, copied next to their generated code in the work directory (`<build>/bench/run`) |

Each benchmark prints `RESULT serialize <ns>`, `RESULT deserialize <ns>` and
any `NOTE <text>` lines; `run.sh` collects them into the table.
