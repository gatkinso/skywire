# Protobuf example (C++ PZ mode)

Sends the `Telemetry` message from skywire to protobuf and back:

1. The skywire C++ `telemetry::Telemetry` serializes into a `Stream` in PZ
   mode, which writes canonical proto3, and the program writes that to the
   file named on its command line.
2. protoc's C++ class for the same message, `telemetry::pb::Telemetry`, parses
   the file, checks the values, and re-encodes it to the same bytes.
3. The program changes two fields on the protobuf side, and the skywire class
   decodes the result through the PZ stream.

Both classes come from the same specs, [../specs](../specs): wiregen writes the
skywire C++ (`--language c++`) and the `.proto` files (`--language proto3`), and
protoc turns those into its own C++.

From the repository root, with `$WIREGEN` the built `wiregen`, `<protobuf>`
where protobuf is installed (`protoc`, its headers and `libprotobuf`) and
`<protozero>` the build's `PROTOZERO_DIR`:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language c++ --write_to_file
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language c++ --write_to_file
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language proto3 --namespace telemetry::pb --write_to_file
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language proto3 --namespace telemetry::pb --write_to_file
<protobuf>/bin/protoc --proto_path=out --cpp_out=out out/Position.proto out/Telemetry.proto
c++ -std=c++17 -Iout -Ilang/cpp -Ilang/c -I<protozero>/include -I<protobuf>/include -o out/example_protobuf \
    examples/protobuf/main.cpp out/Position.pb.cc out/Telemetry.pb.cc <protobuf>/lib/libprotobuf.a
out/example_protobuf out/telemetry.pb
```

A protobuf built with Abseil (version 22 and later) also needs its Abseil
libraries on the link line; with CMake, linking `protobuf::libprotobuf`
brings them, as `examples/CMakeLists.txt` does.

To look at the file, or make it from text:

```
protoc --proto_path=out --decode=telemetry.pb.Telemetry Telemetry.proto < out/telemetry.pb
protoc --proto_path=out --encode=telemetry.pb.Telemetry Telemetry.proto < examples/protobuf/telemetry.txtpb
```

## Using a PZ stream

```cpp
skyloupe::Stream stream(512, 512, skyloupe::Stream::PZ);

sent.serialize(stream);                   // appends one protobuf message
std::string bytes = stream.pz_buffer();   // ...to pz_buffer()
stream.clear();

stream.pz_buffer() = bytes_from_peer;     // one whole protobuf message
received.deserialize(stream);             // decodes all of it, then clears it
```

- Move bytes in and out through `pz_buffer()`, a `std::string`. In PZ mode
  `Stream::read` and `write` read and write a single bytes field, not raw
  bytes.
- Protobuf has no message boundaries, so the stream holds one message at a
  time: `deserialize` decodes the whole buffer as one message. To send several
  over one connection, frame them yourself (e.g. a length before each).
- The bytes are what protobuf itself writes: zero fields are left out, and a
  string goes without its NUL padding (`"SKY1"`, not 8 bytes). So the file is
  52 bytes here, against 43 for the skywire wire format, and its size depends
  on the values.
- Decoding accepts fields in any order, treats missing ones as zero and skips
  unknown ones, as a protobuf peer expects. A bitfield is cut to its bits:
  protobuf sending `status: 11` reads back as 3. A bytes field longer than the
  string member fails, and leaves the message unchanged.
- PZ mode is C++ only. The other languages' generated code doesn't speak
  protobuf; use the `.proto` with that language's protobuf library instead.

See the README's "Protobuf (PZ) mode" section for the type mapping.

## What the build needs

- **Include directories:** `out` (the skywire headers and protoc's `.pb.h`),
  `lang/cpp` and `lang/c` (the skywire runtime), protozero's `include` (the
  PZ stream is built on protozero) and protobuf's `include`.
- **Compile and link:** `main.cpp` and protoc's `Position.pb.cc` and
  `Telemetry.pb.cc`, linked with `libprotobuf`. The skywire side is
  header-only.
- **Lang files:** `--language c++` and `--language proto3` read `lang.cpp.json`
  and `lang.proto3.json` from the directory `wiregen` is in; the build copies
  them there from `lang/cpp/` and `lang/proto/`. `--lang_file` uses another
  copy instead.
- **Namespaces:** `--namespace telemetry::pb` makes the `.proto` package
  `telemetry.pb`, so protoc's classes are in `telemetry::pb` and don't clash
  with skywire's `telemetry::Telemetry` in the same program. When the two
  sides are different programs, generate the `.proto` without it.

`ctest -R skywire_example:Protobuf` runs the same steps and checks the file is
`examples/Telemetry.pb`. It also has protoc encode `telemetry.txtpb` and checks
protoc writes the same bytes.
