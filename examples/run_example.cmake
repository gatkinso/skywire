#
#  Example test driver, run by CTest via `cmake -P` (see CMakeLists.txt).
#
#  Builds one language's example program (README.md), runs it, and checks the
#  file it wrote is byte for byte Telemetry.bin, which every language must
#  produce. The C and C++ examples are built with the project and passed in as
#  EXE. For the others the driver generates Telemetry and Position with wiregen
#  and compiles them with the example, the way the README in each example
#  directory shows. A missing toolchain prints the skip marker the test's
#  SKIP_REGULAR_EXPRESSION matches, the same markers as the round-trip tests.
#
#  The protobuf example (LANG protobuf) writes protobuf, not the skywire wire
#  format, so its EXPECTED is Telemetry.pb. The driver also has protoc encode
#  protobuf/telemetry.txtpb, the same message as text, and checks protoc gets
#  Telemetry.pb too.
#
#  Required -D variables: LANG (c, cpp, protobuf, swift, go, python, java, rust,
#                         csharp),
#                         EXAMPLES_DIR, EXPECTED, OUTDIR
#  For c, cpp, protobuf:  EXE
#  For protobuf:          PROTOC, PROTO_DIR (where the .proto files are)
#  For the others:        WIREGEN, LANG_DIR
#  Optional -D variables: EXE_SUFFIX
#

# Script mode starts with every policy at OLD; match the project's minimum.
cmake_minimum_required(VERSION 3.28)

set(src ${EXAMPLES_DIR}/${LANG})
set(specs ${EXAMPLES_DIR}/specs/Position.json ${EXAMPLES_DIR}/specs/Telemetry.json)

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})

# Generates Position and Telemetry into OUTDIR; extra arguments go to wiregen.
function(generate lang)
    foreach(spec ${specs})
        execute_process(
            COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                    --language ${lang} --write_to_file ${ARGN}
            RESULT_VARIABLE rc)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
        endif()
    endforeach()
endfunction()

function(run_checked what)
    execute_process(${ARGN} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc})")
    endif()
endfunction()

if(LANG STREQUAL "c" OR LANG STREQUAL "cpp")
    set(run ${EXE})

elseif(LANG STREQUAL "protobuf")
    execute_process(
        COMMAND ${PROTOC} --proto_path=${PROTO_DIR} --encode=telemetry.pb.Telemetry Telemetry.proto
        WORKING_DIRECTORY ${PROTO_DIR}
        INPUT_FILE ${src}/telemetry.txtpb
        OUTPUT_FILE ${OUTDIR}/protoc.pb
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc --encode failed (${rc})")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${OUTDIR}/protoc.pb ${EXPECTED}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc encodes telemetry.txtpb differently from ${EXPECTED}")
    endif()
    set(run ${EXE})

elseif(LANG STREQUAL "swift")
    find_program(SWIFTC_EXECUTABLE swiftc)
    if(NOT SWIFTC_EXECUTABLE)
        message("SWIFT_TOOLCHAIN_NOT_FOUND: swiftc not on PATH; skipping the Swift example")
        return()
    endif()
    generate(swift)
    set(exe ${OUTDIR}/example_swift${EXE_SUFFIX})
    run_checked("swiftc" COMMAND ${SWIFTC_EXECUTABLE} -o ${exe} ${LANG_DIR}/swift/SWStream.swift
        ${OUTDIR}/Position.swift ${OUTDIR}/Telemetry.swift ${src}/main.swift)
    set(run ${exe})

elseif(LANG STREQUAL "go")
    find_program(GO_EXECUTABLE go)
    if(NOT GO_EXECUTABLE)
        message("GO_TOOLCHAIN_NOT_FOUND: go not on PATH; skipping the Go example")
        return()
    endif()
    # Never download a newer toolchain to satisfy go.mod.
    set(ENV{GOTOOLCHAIN} local)
    execute_process(COMMAND ${GO_EXECUTABLE} env GOVERSION
        OUTPUT_VARIABLE go_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^go([0-9]+\\.[0-9]+).*" "\\1" go_version_num "${go_version}")
    if(go_version_num VERSION_LESS 1.21)
        message("GO_TOOLCHAIN_NOT_FOUND: ${go_version} is older than go1.21; skipping the Go example")
        return()
    endif()
    # The example is a command, so the messages are generated into package main.
    generate(go --namespace main)
    file(COPY ${src}/main.go DESTINATION ${OUTDIR})
    file(WRITE ${OUTDIR}/go.mod "module wiregen/example\n\ngo 1.21\n")
    get_filename_component(go_bindir ${GO_EXECUTABLE} DIRECTORY)
    find_program(GOFMT_EXECUTABLE gofmt HINTS ${go_bindir})
    if(GOFMT_EXECUTABLE)
        execute_process(COMMAND ${GOFMT_EXECUTABLE} -l .
            WORKING_DIRECTORY ${OUTDIR}
            OUTPUT_VARIABLE unformatted OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(unformatted)
            message(FATAL_ERROR "not gofmt-clean: ${unformatted}")
        endif()
    endif()
    run_checked("go vet" COMMAND ${GO_EXECUTABLE} vet . WORKING_DIRECTORY ${OUTDIR})
    set(exe ${OUTDIR}/example_go${EXE_SUFFIX})
    run_checked("go build" COMMAND ${GO_EXECUTABLE} build -o ${exe} . WORKING_DIRECTORY ${OUTDIR})
    set(run ${exe})

elseif(LANG STREQUAL "python")
    # Windows may have a python3 that only opens the Microsoft Store.
    find_program(PYTHON3_EXECUTABLE NAMES python3 python)
    if(PYTHON3_EXECUTABLE)
        execute_process(COMMAND ${PYTHON3_EXECUTABLE} -c "import sys; print(sys.version_info[0])"
            OUTPUT_VARIABLE py_major OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    if(NOT PYTHON3_EXECUTABLE OR NOT py_major STREQUAL "3")
        message("PYTHON3_NOT_FOUND: no Python 3 interpreter on PATH; skipping the Python example")
        return()
    endif()
    generate(python)
    # Next to the generated modules, so it imports them.
    file(COPY ${src}/main.py DESTINATION ${OUTDIR})
    set(run ${PYTHON3_EXECUTABLE} ${OUTDIR}/main.py)

elseif(LANG STREQUAL "java")
    find_program(JAVAC_EXECUTABLE javac)
    find_program(JAVA_EXECUTABLE java)
    if(JAVAC_EXECUTABLE AND JAVA_EXECUTABLE)
        execute_process(COMMAND ${JAVAC_EXECUTABLE} -version
            RESULT_VARIABLE javac_rc OUTPUT_QUIET ERROR_QUIET)
    endif()
    if(NOT JAVAC_EXECUTABLE OR NOT JAVA_EXECUTABLE OR NOT javac_rc EQUAL 0)
        message("JAVA_TOOLCHAIN_NOT_FOUND: no JDK on PATH; skipping the Java example")
        return()
    endif()
    generate(java)
    run_checked("javac" COMMAND ${JAVAC_EXECUTABLE} -Xlint:all -Werror -d ${OUTDIR}/classes
        ${OUTDIR}/Position.java ${OUTDIR}/Telemetry.java ${src}/Main.java)
    set(run ${JAVA_EXECUTABLE} -cp ${OUTDIR}/classes Main)

elseif(LANG STREQUAL "rust")
    # rustup installs into ~/.cargo/bin, which a non-interactive shell may lack.
    set(cargo_bins $ENV{HOME}/.cargo/bin $ENV{USERPROFILE}/.cargo/bin)
    find_program(RUSTC_EXECUTABLE rustc HINTS ${cargo_bins})
    if(NOT RUSTC_EXECUTABLE)
        message("RUST_TOOLCHAIN_NOT_FOUND: rustc not on PATH or in ~/.cargo/bin; skipping the Rust example")
        return()
    endif()
    execute_process(COMMAND ${RUSTC_EXECUTABLE} --version
        OUTPUT_VARIABLE rustc_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^rustc ([0-9]+\\.[0-9]+).*" "\\1" rustc_version_num "${rustc_version}")
    if(rustc_version_num VERSION_LESS 1.58)
        message("RUST_TOOLCHAIN_NOT_FOUND: ${rustc_version} is older than rustc 1.58; skipping the Rust example")
        return()
    endif()
    generate(rust)
    # The crate root, next to the modules it declares.
    file(COPY ${src}/main.rs DESTINATION ${OUTDIR})
    get_filename_component(rust_bindir ${RUSTC_EXECUTABLE} DIRECTORY)
    find_program(RUSTFMT_EXECUTABLE rustfmt HINTS ${rust_bindir} ${cargo_bins})
    if(RUSTFMT_EXECUTABLE)
        run_checked("rustfmt --check" COMMAND ${RUSTFMT_EXECUTABLE} --edition 2021 --check main.rs
            WORKING_DIRECTORY ${OUTDIR})
    endif()
    set(exe ${OUTDIR}/example_rust${EXE_SUFFIX})
    run_checked("rustc" COMMAND ${RUSTC_EXECUTABLE} --edition 2021 -D warnings -o ${exe} main.rs
        WORKING_DIRECTORY ${OUTDIR})
    find_program(CLIPPY_EXECUTABLE clippy-driver HINTS ${rust_bindir} ${cargo_bins})
    if(CLIPPY_EXECUTABLE)
        run_checked("clippy" COMMAND ${CLIPPY_EXECUTABLE} --edition 2021 -D warnings
            -o ${OUTDIR}/clippy_example${EXE_SUFFIX} main.rs
            WORKING_DIRECTORY ${OUTDIR})
    endif()
    set(run ${exe})

elseif(LANG STREQUAL "csharp")
    find_program(DOTNET_EXECUTABLE dotnet)
    if(DOTNET_EXECUTABLE)
        # Only the runtime, not an SDK, may be installed.
        execute_process(COMMAND ${DOTNET_EXECUTABLE} --version
            RESULT_VARIABLE dotnet_rc OUTPUT_VARIABLE dotnet_version ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    if(NOT DOTNET_EXECUTABLE OR NOT dotnet_rc EQUAL 0 OR NOT dotnet_version MATCHES "^[0-9]+\\.")
        message("CSHARP_TOOLCHAIN_NOT_FOUND: no .NET SDK on PATH; skipping the C# example")
        return()
    endif()
    set(ENV{DOTNET_CLI_TELEMETRY_OPTOUT} 1)
    set(ENV{DOTNET_NOLOGO} 1)
    set(ENV{DOTNET_SKIP_FIRST_TIME_EXPERIENCE} 1)
    generate(csharp)
    # The project builds every .cs file in its directory.
    file(COPY ${src}/Program.cs ${src}/Example.csproj DESTINATION ${OUTDIR})
    run_checked("dotnet build" COMMAND ${DOTNET_EXECUTABLE} build -c Release -o ${OUTDIR}/bin
        WORKING_DIRECTORY ${OUTDIR})
    set(run ${DOTNET_EXECUTABLE} ${OUTDIR}/bin/Example.dll)

else()
    message(FATAL_ERROR "unknown LANG '${LANG}'")
endif()

set(written ${OUTDIR}/telemetry.bin)
run_checked("the ${LANG} example" COMMAND ${run} ${written} WORKING_DIRECTORY ${OUTDIR})

execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${written} ${EXPECTED}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    file(READ ${written} got HEX)
    file(READ ${EXPECTED} want HEX)
    message(FATAL_ERROR "the ${LANG} example wrote different bytes:\n"
                        "  got  ${got}\n  want ${want}")
endif()
