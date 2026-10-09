#
#  Swift round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the Swift toolchain at test time. If swiftc is absent it prints the
#  skip marker matched by the test's SKIP_REGULAR_EXPRESSION; otherwise it
#  generates the Swift sources with wiregen, compiles them and runs the result.
#
#  Required -D variables: WIREGEN, SPEC, LANGFILE, RUNTIME, OUTDIR
#  Optional -D variables: EXE_SUFFIX
#

find_program(SWIFTC_EXECUTABLE swiftc)

if(NOT SWIFTC_EXECUTABLE)
    message("SWIFT_TOOLCHAIN_NOT_FOUND: swiftc not on PATH; skipping Swift round-trip test")
    return()
endif()

message("Using swiftc: ${SWIFTC_EXECUTABLE}")

file(MAKE_DIRECTORY ${OUTDIR})
get_filename_component(classname ${SPEC} NAME_WE)
set(impl ${OUTDIR}/${classname}.swift)
set(test ${OUTDIR}/${classname}_test.swift)
set(bin  ${OUTDIR}/swifty_test${EXE_SUFFIX})

execute_process(
    COMMAND ${WIREGEN} --input_file ${SPEC} --outdir ${OUTDIR}
            --langfile ${LANGFILE} --generate_tests --write_to_file
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "wiregen failed (${rc})")
endif()

execute_process(
    COMMAND ${SWIFTC_EXECUTABLE} -o ${bin} ${RUNTIME} ${impl} ${test}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "swiftc failed (${rc})")
endif()

execute_process(COMMAND ${bin} RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Swift round-trip failed (${rc})")
endif()
