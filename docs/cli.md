# Running wiregen

    mkdir -p out
    wiregen --language c++ --input_file specs/MyClass.json --outdir out --write_to_file

The message specification does not say which language to generate; the command
line does. `--language` takes one of `c`, `c++` (or `cpp`), `csharp` (or `cs`,
`c#`), `go` (or `golang`), `java`, `javascript` (or `js`),
`proto3`, `python3` (or `python`), `rust` and `swift`, and uses that language's lang
file from wiregen's own directory or, in an install, from `share/skywire/lang`. `--lang_file <path>` uses a
different [lang file](lang-files.md) instead (for example a customized copy); on its own it also
sets the language, since every lang file names its language. If both are given
they must agree. (`--langfile` is the old spelling of `--lang_file`.)

`--namespace <name>` replaces the specification's `namespace` (the C++
namespace, the C# namespace, or the Go, Java or proto3 package), so one
specification can be generated into different namespaces. The specification
need not have a namespace at all when the command line gives one. C, Python,
Rust, Swift and JavaScript have no use for it: a Python, Rust or JavaScript
module's name is its file, C has no namespaces, and Swift's are modules.

Without `--write_to_file` the generated code is printed to standard output.
With it, the files go in `--outdir`, which must already exist.

## Options

| Option | Effect |
|---|---|
| `--input_file <file>` | The message specification to read. |
| `--input_str <json>` | The message specification, given on the command line instead. |
| `--language <name>` | The language to generate, using its installed lang file (see above): `c`, `c++` (or `cpp`), `csharp` (or `cs`, `c#`), `go` (or `golang`), `java`, `javascript` (or `js`), `proto3`, `python3` (or `python`), `rust`, `swift`. |
| `--lang_file <file>` | A lang file to use instead of the installed one (`--langfile` is the old spelling). |
| `--langspec <json>` | Lang file contents, given on the command line instead. |
| `--namespace <name>` | The namespace (package) to generate into, replacing the specification's. |
| `--generate_tests` | Also generate unit tests (see [Lang files](lang-files.md#tests-and-relays)). The C and C++ tests need Google Test (see [Using skywire](using.md#generated-tests)). |
| `--generate_relay` | Also generate a relay program (see [Lang files](lang-files.md#tests-and-relays)). |
| `--generate_json` | Also generate `to_json` and `from_json` (see [JSON, XML and BSON](text-formats.md)). C, C++, Python, Go, C# and Rust only for now. |
| `--generate_xml` | Also generate `to_xml` and `from_xml` (see [JSON, XML and BSON](text-formats.md)). C, C++, Python, Go, C# and Rust only for now. |
| `--generate_bson` | Also generate `to_bson` and `from_bson` (see [JSON, XML and BSON](text-formats.md#bson)). C, C++, Python, Go, C# and Rust only for now. |
| `--write_to_file` | Write the generated files. Without it they are printed to standard output. |
| `--outdir <dir>` | The directory to write the files to (default: the current directory). |
| `--silent` | Do not print the success or failure summary. |
| `--banner` | Print a banner with wiregen's version and how it was built (commit, branch, compiler, build type, platform), then carry on. On its own, print it and exit. |
| `--version` | Print wiregen's version and exit. |
| `--help` | Print the options and exit. |

If an option is given twice, the last one wins. An unknown option, an option missing its
value, or a value given to an option that takes none is an error, and wiregen
generates nothing. With no arguments wiregen prints its options. On failure the
exit status is non-zero.

## Version and banner

The version is the `VERSION` in the top-level `CMakeLists.txt`; the library
reports it as `skyloupe::version()` (and `version_major()`, `version_minor()`,
`version_patch()`). The revision `--banner` shows is `git describe --tags
--always --dirty` of the source tree, taken at build time, so it follows
commits without reconfiguring; the library reports it as `skyloupe::revision()`.
When building outside a git checkout (a source tarball, a package) it is empty
unless you give one with `-DSKYWIRE_REVISION=<revision>`. The rest of the
banner — full commit hash and date, branch, compiler, build type, platform and
whether sanitizers are on — is `skyloupe::build_info()`. There is no build
timestamp, so rebuilding unchanged sources rebuilds nothing.
