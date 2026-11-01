# Homebrew

The formula installs a release archive as it is: `wiregen`, the runtime
headers, the lang files, the CMake package and `skywire.pc`. It depends on
`protozero`, which generated C++ needs. It is for Apple silicon Macs only:
there is no macOS x86_64 archive, and on Linux the release tarball needs
nothing installed.

It lives in the tap `gatkinso/homebrew-tap` as `Formula/skywire.rb`:

```
brew install gatkinso/tap/skywire
```

| Script | Purpose |
|---|---|
| `make_formula.py` | Writes `skywire.rb` for a release from its `SHA256SUMS`. |
| `test_formula.py` | Installs the formula from a local archive through a scratch tap, runs `brew test` and `brew audit`, and builds `src/test/install/consumer` against it. |

## Releasing

After the release workflow publishes `vX.Y.Z`:

```
gh release download vX.Y.Z -p SHA256SUMS -D /tmp/sums
python3 packaging/homebrew/make_formula.py X.Y.Z /tmp/sums/SHA256SUMS \
    -o ../homebrew-tap/Formula/skywire.rb
```

and commit it to the tap. The release workflow has already installed and
tested the formula on its macOS smoke runner, from the archive it built, but
without `brew audit`; run `test_formula.py` locally for that.

The downloads need a public repository: while skywire is private, `brew
install` from the tap fails with a 404.

## Testing locally

```
gh release download v0.9.0 -p '*-macos-arm64.tar.gz' -D /tmp/dist
python3 packaging/homebrew/test_formula.py /tmp/dist --version 0.9.0
```

It needs an Apple silicon Mac with `brew`, `cmake` and Xcode's command line
tools, and skywire not already installed. It makes the tap
`skywire-test/local` and removes it afterwards (`--keep` leaves it, with
skywire installed), and turns Homebrew's developer mode back off if
`brew tap-new` turned it on. `--no-audit` skips `brew audit`, whose first run
installs Homebrew's Ruby gems.
