# C# example

Fills a `Telemetry` message (with its nested `Position`), serializes it to a
file, reads the file back, and checks the message read back equals the one
written; then does the same with its JSON, XML and BSON forms
(`telemetry.json`, `telemetry.xml` and `telemetry.bson` beside the file; see
[JSON, XML and BSON](../../docs/text-formats.md)). See
[../README.md](../README.md) for the messages.

From the repository root, with `$WIREGEN` the built `wiregen` and the .NET SDK
installed:

```sh
mkdir -p out
$WIREGEN --input_file examples/specs/Position.json  --outdir out --language csharp --write_to_file \
    --generate_json --generate_xml --generate_bson
$WIREGEN --input_file examples/specs/Telemetry.json --outdir out --language csharp --write_to_file \
    --generate_json --generate_xml --generate_bson
cp examples/csharp/Program.cs examples/csharp/Example.csproj out/
dotnet run --project out -- out/telemetry.bin
```

The messages are in namespace `telemetry`, the specs' namespace; `Program.cs`
has `using telemetry;`.

## What the build needs

- **Include directories:** None. `Example.csproj` compiles every `.cs` file in
  its directory, so the generated files go next to it (here, the two it needs
  are copied into `out/`).
- **Compile and link:** The generated `Position.cs` and `Telemetry.cs`,
  `SkywireText.cs` (the JSON, XML and BSON helpers, which wiregen writes beside
  them) and `Program.cs`, in one project. They use only the base class library; there is
  no runtime package. The project targets the framework the installed SDK
  ships with, at C# 7.3, with warnings as errors. The generated code is plain
  C# 7.3, so it also builds in a .NET Framework or Unity project.
- **Lang file:** `--language csharp` reads `lang.csharp.json` from the
  directory `wiregen` is in; the build copies it there from `lang/csharp/`. To
  use the source copy, or a customized one, pass
  `--lang_file lang/csharp/lang.csharp.json` instead of `--language`. The lang
  file sets the generated file names.

`ctest -R skywire_example:CSharp` runs the same steps (with `dotnet build`,
then running the built program) and checks the files are
`examples/Telemetry.bin`, `.json`, `.xml` and `.bson`, which every language
writes.
