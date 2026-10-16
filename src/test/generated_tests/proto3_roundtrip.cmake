#
#  proto3 wire-format test driver, run by CTest via `cmake -P`.
#
#  Generates the .proto and the text-format test message with wiregen, then
#  checks with protoc that the .proto compiles, that the test message encodes
#  to the bytes wiregen computed (the "# expected:" line, which is also what the
#  C++ PZ stream must produce), and that decoding and re-encoding gives the same
#  bytes.
#
#  Required -D variables: WIREGEN, PROTOC, SPEC, LANGFILE, OUTDIR
#

file(MAKE_DIRECTORY ${OUTDIR})
get_filename_component(classname ${SPEC} NAME_WE)

execute_process(
    COMMAND ${WIREGEN} --input_file ${SPEC} --outdir ${OUTDIR}
            --lang_file ${LANGFILE} --generate_tests --write_to_file
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "wiregen failed (${rc})")
endif()

set(proto ${classname}.proto)
set(txtpb ${OUTDIR}/${classname}Test.txtpb)

execute_process(
    COMMAND ${PROTOC} -I. --descriptor_set_out=${classname}.desc ${proto}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "protoc rejected the generated ${proto} (${rc})")
endif()

file(STRINGS ${OUTDIR}/${proto} package_line REGEX "^package ")
string(REGEX REPLACE "^package ([A-Za-z0-9_.]+).*" "\\1." package "${package_line}")
set(message_type ${package}${classname})

file(STRINGS ${txtpb} expected_line REGEX "^# expected: ")
string(REGEX REPLACE "^# expected: ([0-9a-f]*).*" "\\1" expected "${expected_line}")

execute_process(
    COMMAND ${PROTOC} -I. --encode=${message_type} ${proto}
    WORKING_DIRECTORY ${OUTDIR}
    INPUT_FILE ${txtpb}
    OUTPUT_FILE ${OUTDIR}/${classname}.bin
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "protoc --encode failed (${rc})")
endif()

file(READ ${OUTDIR}/${classname}.bin got HEX)
if(NOT got STREQUAL expected)
    message(FATAL_ERROR "wire mismatch\n got ${got}\nwant ${expected}")
endif()
message("protoc encoding matches the expected ${classname} bytes")

execute_process(
    COMMAND ${PROTOC} -I. --decode=${message_type} ${proto}
    WORKING_DIRECTORY ${OUTDIR}
    INPUT_FILE ${OUTDIR}/${classname}.bin
    OUTPUT_FILE ${OUTDIR}/${classname}.decoded.txtpb
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "protoc --decode failed (${rc})")
endif()

execute_process(
    COMMAND ${PROTOC} -I. --encode=${message_type} ${proto}
    WORKING_DIRECTORY ${OUTDIR}
    INPUT_FILE ${OUTDIR}/${classname}.decoded.txtpb
    OUTPUT_FILE ${OUTDIR}/${classname}.again.bin
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "protoc --encode of the decoded message failed (${rc})")
endif()

file(READ ${OUTDIR}/${classname}.again.bin again HEX)
if(NOT again STREQUAL expected)
    message(FATAL_ERROR "decode/encode round trip changed the bytes\n got ${again}\nwant ${expected}")
endif()
