# Security policy

## Reporting a vulnerability

Please report security problems privately, not in a public issue: use
**Report a vulnerability** on this repository's
[Security tab](https://github.com/gatkinso/skywire/security/advisories/new)
(GitHub's private vulnerability reporting). Include what is affected, how
to reproduce it (a spec, a lang file or an input that shows it), and what
you think the impact is. You can expect an answer within a week, and a fix,
or a plan for one, before anything is made public.

## What is in scope

- **The code wiregen generates**, in every language: its readers take
  untrusted input. That is `deserialize`, protobuf (PZ) decoding in C++, and
  with `--generate_json`, `--generate_xml` or `--generate_bson` the JSON, XML
  and BSON readers. Any input that crashes them, reads or writes outside a
  buffer, loops without end, or exhausts memory or the stack is a
  vulnerability; input that is merely refused is not.
- **The runtimes** wiregen's output uses: `lang/c`, `lang/cpp`,
  `lang/swift`, and the helper files wiregen writes for the other languages
  (`lang/*/skywire_text.*`, `SkywireText.*`).
- **wiregen itself**, when given a spec or lang file from someone else.

The generated readers are fuzzed (`src/test/fuzz`) and differentially
tested against the C++ ones (`skywire_text_diff`); a reproducer from either
is the most useful report.

## Supported versions

Fixes go into the next release. Only the latest release is supported.
