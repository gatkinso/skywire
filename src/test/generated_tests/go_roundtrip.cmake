#
#  Go round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the Go toolchain at test time. If go is absent (or older than 1.21,
#  which added binary.NativeEndian) it prints the skip marker matched by the
#  test's SKIP_REGULAR_EXPRESSION; otherwise it generates the Go sources with
#  wiregen (with the JSON, XML and BSON forms), checks they are gofmt-clean,
#  and runs `go vet` and `go test`.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

find_program(GO_EXECUTABLE go)

if(NOT GO_EXECUTABLE)
    message("GO_TOOLCHAIN_NOT_FOUND: go not on PATH; skipping Go round-trip test")
    return()
endif()

# Never download a newer toolchain to satisfy go.mod.
set(ENV{GOTOOLCHAIN} local)

execute_process(COMMAND ${GO_EXECUTABLE} env GOVERSION
    OUTPUT_VARIABLE go_version OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "^go([0-9]+\\.[0-9]+).*" "\\1" go_version_num "${go_version}")
if(go_version_num VERSION_LESS 1.21)
    message("GO_TOOLCHAIN_NOT_FOUND: ${go_version} is older than go1.21; skipping Go round-trip test")
    return()
endif()

message("Using ${GO_EXECUTABLE} (${go_version})")

file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")
file(WRITE ${OUTDIR}/go.mod "module wiregen/generated\n\ngo 1.21\n")

foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
                --generate_json --generate_xml --generate_bson
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

get_filename_component(go_bindir ${GO_EXECUTABLE} DIRECTORY)
find_program(GOFMT_EXECUTABLE gofmt HINTS ${go_bindir})
if(GOFMT_EXECUTABLE)
    execute_process(COMMAND ${GOFMT_EXECUTABLE} -l .
        WORKING_DIRECTORY ${OUTDIR}
        OUTPUT_VARIABLE unformatted OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(unformatted)
        message(FATAL_ERROR "generated Go is not gofmt-clean: ${unformatted}")
    endif()
endif()

foreach(step vet test)
    if(step STREQUAL "test")
        set(args test -count=1 -v ./...)
    else()
        set(args vet ./...)
    endif()

    execute_process(COMMAND ${GO_EXECUTABLE} ${args}
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "go ${step} failed (${rc})")
    endif()
endforeach()
