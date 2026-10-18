#
#  Rust round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the Rust toolchain at test time. If rustc is absent, or older than
#  1.58 (edition 2021, and the inline format arguments the relays use), it
#  prints the skip marker matched by the test's SKIP_REGULAR_EXPRESSION;
#  otherwise it generates the Rust modules and their tests with wiregen,
#  checks they are rustfmt-clean, and builds and runs them as one crate with
#  `rustc --test -D warnings`. When clippy-driver is
#  present the crate must also be clippy-clean.
#
#  Plain rustc, no Cargo: the generated code has no dependencies, and the
#  driver writes the crate root (lib.rs) declaring each generated module.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  Optional -D variables: EXE_SUFFIX
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

# rustup installs into ~/.cargo/bin, which a non-interactive shell (ssh, an
# IDE, CI) may not have on PATH.
set(cargo_bins $ENV{HOME}/.cargo/bin $ENV{USERPROFILE}/.cargo/bin)
find_program(RUSTC_EXECUTABLE rustc HINTS ${cargo_bins})

if(NOT RUSTC_EXECUTABLE)
    message("RUST_TOOLCHAIN_NOT_FOUND: rustc not on PATH or in ~/.cargo/bin; skipping Rust round-trip test")
    return()
endif()

execute_process(COMMAND ${RUSTC_EXECUTABLE} --version
    OUTPUT_VARIABLE rustc_version OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "^rustc ([0-9]+\\.[0-9]+).*" "\\1" rustc_version_num "${rustc_version}")
if(rustc_version_num VERSION_LESS 1.58)
    message("RUST_TOOLCHAIN_NOT_FOUND: ${rustc_version} is older than rustc 1.58; skipping Rust round-trip test")
    return()
endif()

message("Using ${RUSTC_EXECUTABLE} (${rustc_version})")

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")

foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

# The crate root: every generated file is a module at the top of the crate,
# so messages and tests reach each other as super::<module>.
file(GLOB generated RELATIVE ${OUTDIR} ${OUTDIR}/*.rs)
list(SORT generated)
set(crate_root "// Written by rust_roundtrip.cmake.\n\n")
foreach(file ${generated})
    string(REGEX REPLACE "\\.rs$" "" module ${file})
    string(APPEND crate_root "pub mod ${module};\n")
endforeach()
file(WRITE ${OUTDIR}/lib.rs "${crate_root}")

get_filename_component(rust_bindir ${RUSTC_EXECUTABLE} DIRECTORY)
find_program(RUSTFMT_EXECUTABLE rustfmt HINTS ${rust_bindir} ${cargo_bins})
if(RUSTFMT_EXECUTABLE)
    execute_process(COMMAND ${RUSTFMT_EXECUTABLE} --edition 2021 --check ${generated}
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "generated Rust is not rustfmt-clean:\n${out}")
    endif()
endif()

set(runner ${OUTDIR}/wiregen_tests${EXE_SUFFIX})
execute_process(
    COMMAND ${RUSTC_EXECUTABLE} --edition 2021 --test -D warnings -o ${runner} lib.rs
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "rustc failed (${rc})")
endif()

find_program(CLIPPY_EXECUTABLE clippy-driver HINTS ${rust_bindir} ${cargo_bins})
if(CLIPPY_EXECUTABLE)
    execute_process(
        COMMAND ${CLIPPY_EXECUTABLE} --edition 2021 --test -D warnings -o ${OUTDIR}/clippy_tests${EXE_SUFFIX} lib.rs
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "clippy failed (${rc})")
    endif()
endif()

execute_process(COMMAND ${runner}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Rust tests failed (${rc})")
endif()
