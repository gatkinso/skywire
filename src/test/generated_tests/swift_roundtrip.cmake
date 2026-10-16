#
#  Swift round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the Swift toolchain at test time. If swiftc is absent it prints the
#  skip marker matched by the test's SKIP_REGULAR_EXPRESSION; otherwise it
#  generates the Swift sources with wiregen, compiles them and runs the result.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, RUNTIME, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#  Each test has its own @main, so each is built separately, with every
#  message source.
#  Optional -D variables: EXE_SUFFIX
#

find_program(SWIFTC_EXECUTABLE swiftc)

if(NOT SWIFTC_EXECUTABLE)
    message("SWIFT_TOOLCHAIN_NOT_FOUND: swiftc not on PATH; skipping Swift round-trip test")
    return()
endif()

message("Using swiftc: ${SWIFTC_EXECUTABLE}")

file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")

set(classnames "")
set(impls "")
foreach(spec ${SPECS})
    get_filename_component(classname ${spec} NAME_WE)
    list(APPEND classnames ${classname})
    list(APPEND impls ${OUTDIR}/${classname}.swift)

    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

foreach(classname ${classnames})
    set(bin ${OUTDIR}/${classname}_test${EXE_SUFFIX})

    execute_process(
        COMMAND ${SWIFTC_EXECUTABLE} -o ${bin} ${RUNTIME} ${impls} ${OUTDIR}/${classname}_test.swift
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "swiftc failed on ${classname} (${rc})")
    endif()

    execute_process(COMMAND ${bin} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Swift round-trip ${classname} failed (${rc})")
    endif()
endforeach()
