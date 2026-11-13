#
#  Python 3 round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects a Python 3 interpreter at test time. If none is found it prints the
#  skip marker matched by the test's SKIP_REGULAR_EXPRESSION; otherwise it
#  generates the Python sources with wiregen, byte-compiles them, and runs the
#  generated unittest module.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

find_program(PYTHON3_EXECUTABLE NAMES python3 python)

if(PYTHON3_EXECUTABLE)
    execute_process(COMMAND ${PYTHON3_EXECUTABLE} -c "import sys; print(sys.version_info[0])"
        OUTPUT_VARIABLE py_major OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT PYTHON3_EXECUTABLE OR NOT py_major STREQUAL "3")
    message("PYTHON3_NOT_FOUND: no Python 3 interpreter on PATH; skipping Python round-trip test")
    return()
endif()

message("Using ${PYTHON3_EXECUTABLE}")

file(MAKE_DIRECTORY ${OUTDIR})
string(REPLACE "," ";" SPECS "${SPECS}")

foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests
                --generate_json --generate_xml --generate_bson --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

set(modules "")
set(sources skywire_text.py)
foreach(spec ${SPECS})
    get_filename_component(classname ${spec} NAME_WE)
    list(APPEND modules ${classname}Test)
    list(APPEND sources ${classname}.py ${classname}Test.py)
endforeach()

execute_process(
    COMMAND ${PYTHON3_EXECUTABLE} -m py_compile ${sources}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "generated Python does not compile (${rc})")
endif()

execute_process(
    COMMAND ${PYTHON3_EXECUTABLE} -m unittest -v ${modules}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Python round-trip failed (${rc})")
endif()
