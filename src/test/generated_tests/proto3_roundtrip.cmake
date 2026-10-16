#
#  proto3 wire-format test driver, run by CTest via `cmake -P`.
#
#  Generates each spec's .proto and text-format test message with wiregen, then
#  checks with protoc that the .proto compiles, that the test message encodes
#  to the bytes wiregen computed (the "# expected:" line, which is also what the
#  C++ PZ stream must produce), and that decoding and re-encoding gives the same
#  bytes.
#
#  SAMPLES are hand-written text-format messages, named <message>.<anything>.txtpb,
#  each with its own "# expected:" line; they get the same encode and decode
#  checks. They cover what generated tests can't, such as nested messages with
#  values set.
#
#  Required -D variables: WIREGEN, PROTOC, SPECS, LANGFILE, OUTDIR
#  (SPECS and SAMPLES are comma-separated, since CTest splits arguments on ';').
#  Optional: SAMPLES
#

file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")
string(REPLACE "," ";" SAMPLES "${SAMPLES}")

# Encodes txtpb as the named message, compares with its "# expected:" line,
# then decodes and re-encodes and compares again.
function(check_encoding classname txtpb)
    set(proto ${classname}.proto)

    file(STRINGS ${OUTDIR}/${proto} package_line REGEX "^package ")
    string(REGEX REPLACE "^package ([A-Za-z0-9_.]+).*" "\\1." package "${package_line}")
    set(message_type ${package}${classname})

    file(STRINGS ${txtpb} expected_line REGEX "^# expected: ")
    string(REGEX REPLACE "^# expected: ([0-9a-f]*).*" "\\1" expected "${expected_line}")
    get_filename_component(name ${txtpb} NAME_WE)

    execute_process(
        COMMAND ${PROTOC} -I. --encode=${message_type} ${proto}
        WORKING_DIRECTORY ${OUTDIR}
        INPUT_FILE ${txtpb}
        OUTPUT_FILE ${OUTDIR}/${name}.bin
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc --encode of ${txtpb} failed (${rc})")
    endif()

    file(READ ${OUTDIR}/${name}.bin got HEX)
    if(NOT got STREQUAL expected)
        message(FATAL_ERROR "${txtpb}: wire mismatch\n got ${got}\nwant ${expected}")
    endif()
    message("protoc encoding of ${txtpb} matches the expected ${classname} bytes")

    execute_process(
        COMMAND ${PROTOC} -I. --decode=${message_type} ${proto}
        WORKING_DIRECTORY ${OUTDIR}
        INPUT_FILE ${OUTDIR}/${name}.bin
        OUTPUT_FILE ${OUTDIR}/${name}.decoded.txtpb
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc --decode of ${txtpb} failed (${rc})")
    endif()

    execute_process(
        COMMAND ${PROTOC} -I. --encode=${message_type} ${proto}
        WORKING_DIRECTORY ${OUTDIR}
        INPUT_FILE ${OUTDIR}/${name}.decoded.txtpb
        OUTPUT_FILE ${OUTDIR}/${name}.again.bin
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc --encode of the decoded ${txtpb} failed (${rc})")
    endif()

    file(READ ${OUTDIR}/${name}.again.bin again HEX)
    if(NOT again STREQUAL expected)
        message(FATAL_ERROR "${txtpb}: decode/encode round trip changed the bytes\n got ${again}\nwant ${expected}")
    endif()
endfunction()

# Generate every .proto first: a message's imports must exist before protoc reads it.
foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

foreach(spec ${SPECS})
    get_filename_component(classname ${spec} NAME_WE)
    set(proto ${classname}.proto)

    execute_process(
        COMMAND ${PROTOC} -I. --descriptor_set_out=${classname}.desc ${proto}
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "protoc rejected the generated ${proto} (${rc})")
    endif()

    check_encoding(${classname} ${OUTDIR}/${classname}Test.txtpb)
endforeach()

foreach(sample ${SAMPLES})
    get_filename_component(name ${sample} NAME)
    string(REGEX REPLACE "^([A-Za-z0-9_]+)\\..*" "\\1" classname "${name}")
    check_encoding(${classname} ${sample})
endforeach()
