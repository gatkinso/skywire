#
#  Swift round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the Swift toolchain at test time. If swiftc is absent it prints the
#  skip marker matched by the test's SKIP_REGULAR_EXPRESSION; otherwise it
#  generates the Swift sources with wiregen (with the JSON, XML and BSON
#  forms), compiles them and runs the result.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, RUNTIME, OUTDIR
#  SPECS and RUNTIME are comma-separated lists (CTest splits arguments on
#  ';'); a spec's nested message types must be among SPECS.
#  Each test has its own @main, so each is built separately, with the runtime,
#  its own message and the messages nested in it. swiftc starts from scratch
#  every time, so the builds run at once.
#  Optional -D variables: EXE_SUFFIX
#

# Script mode starts with every policy at OLD; match the project's minimum so
# IN_LIST and the other newer features behave the same on every CMake version.
cmake_minimum_required(VERSION 3.28)

find_program(SWIFTC_EXECUTABLE swiftc)

if(NOT SWIFTC_EXECUTABLE)
    message("SWIFT_TOOLCHAIN_NOT_FOUND: swiftc not on PATH; skipping Swift round-trip test")
    return()
endif()

message("Using swiftc: ${SWIFTC_EXECUTABLE}")

file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")
string(REPLACE "," ";" RUNTIME "${RUNTIME}")

# A member type that isn't one of the lang file's native types names a
# nested message.
file(READ ${LANGFILE} lang_json)
string(JSON native_count LENGTH "${lang_json}" native_types)
set(native_types "")
math(EXPR last "${native_count} - 1")
foreach(i RANGE ${last})
    string(JSON type GET "${lang_json}" native_types ${i})
    list(APPEND native_types "${type}")
endforeach()

set(classnames "")
foreach(spec ${SPECS})
    get_filename_component(classname ${spec} NAME_WE)
    list(APPEND classnames ${classname})

    # The message types this one contains directly.
    file(READ ${spec} spec_json)
    string(JSON member_count LENGTH "${spec_json}" members)
    set(nested_${classname} "")
    math(EXPR last "${member_count} - 1")
    foreach(i RANGE ${last})
        string(JSON tag MEMBER "${spec_json}" members ${i})
        string(JSON type GET "${spec_json}" members ${tag} type)
        if(NOT type IN_LIST native_types)
            list(APPEND nested_${classname} ${type})
        endif()
    endforeach()

    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
                --generate_json --generate_xml --generate_bson
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

# Sets out to classname and every message nested in it, at any depth.
function(with_nested classname out)
    set(found ${classname})
    set(pending ${classname})
    while(pending)
        list(POP_FRONT pending current)
        foreach(type ${nested_${current}})
            if(NOT type IN_LIST classnames)
                message(FATAL_ERROR "${current} contains ${type}, which is not among SPECS")
            endif()
            if(NOT type IN_LIST found)
                list(APPEND found ${type})
                list(APPEND pending ${type})
            endif()
        endforeach()
    endwhile()
    set(${out} ${found} PARENT_SCOPE)
endfunction()

# execute_process runs all its COMMANDs at the same time (as a pipeline;
# swiftc reads no input, so nothing passes between them).
set(builds "")
foreach(classname ${classnames})
    with_nested(${classname} messages)
    set(sources ${RUNTIME})
    foreach(message ${messages})
        list(APPEND sources ${OUTDIR}/${message}.swift)
    endforeach()
    list(APPEND builds COMMAND ${SWIFTC_EXECUTABLE} -o ${OUTDIR}/${classname}_test${EXE_SUFFIX}
                               ${sources} ${OUTDIR}/${classname}_test.swift)
endforeach()

execute_process(${builds}
    RESULTS_VARIABLE results
    OUTPUT_VARIABLE out ERROR_VARIABLE out)
set(failed "")
foreach(classname rc IN ZIP_LISTS classnames results)
    if(NOT rc EQUAL 0)
        list(APPEND failed "${classname} (${rc})")
    endif()
endforeach()
if(failed)
    list(JOIN failed ", " failed)
    message(FATAL_ERROR "swiftc failed on ${failed}:\n${out}")
endif()

foreach(classname ${classnames})
    execute_process(COMMAND ${OUTDIR}/${classname}_test${EXE_SUFFIX} RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Swift round-trip ${classname} failed (${rc})")
    endif()
endforeach()
