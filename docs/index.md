# Skywire

Skywire is a simple yet surprisingly versatile message serialization generator
for C, C++, C#, Go, Java, JavaScript, Python, Rust and Swift.

Based on Google Protocol Buffer technology, Skywire takes protocol buffers to
a place it fears to tread: namely, the ability to define a wire protocol.

Yes, that's right. Bits. Endianness. Terrible things to behold. Oh my.

Unlike generators like Protocol Buffers, the Skywire generator (known as
"wiregen") is asymmetric: wiregen generated code only has to run on one side
of the connection. If you have an ICD for the component or device you are
trying to communicate with, wiregen lets you define a "stencil" for a message
(a C++ class, a C struct, a Go struct, ...) and generates the code to talk to
it.

A [specification](specs.md) is a JSON file listing the message's members:
each one's tag, type, name and byte order, plus fixed-length strings,
bitfields and nested messages. It says nothing about the language; the command
line picks that, and a [lang file](lang-files.md) per language holds the
language's conventions. Every language puts the same bytes on the wire, and
the [tests](testing.md) check that they do. wiregen can also write the
matching proto3 `.proto`, and generated C++ can travel as protobuf
([PZ mode](protobuf.md)).

You have wide latitude in the manner and style of the generated code, so it
can follow in-house naming and style conventions. wiregen can also generate
unit tests for the generated code, and a "relay" program that checks a
message read from a file member by member. While these are not needed with
the default lang files, they come in very handy once you customize them.

## Start here

- [Quick start](quick-start.md): from an ICD table to working C in four steps.
- [Why skywire?](why.md): what it's for, what it isn't, and the alternatives.
- [Specifications](specs.md): the JSON format.
- [Installing](install.md): release archives, Homebrew, vcpkg, Conan.

Skywire is open source under the Apache License 2.0; the code it generates is
yours under any license ([License](license.md)). The source is at
[github.com/gatkinso/skywire](https://github.com/gatkinso/skywire).
