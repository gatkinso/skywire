#
#  Java round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects a JDK at test time. If javac is absent (or is the macOS stub that
#  only prompts to install one) it prints the skip marker matched by the test's
#  SKIP_REGULAR_EXPRESSION; otherwise it generates the Java sources with
#  wiregen, compiles them with all lint warnings as errors, and runs the
#  generated test class.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

find_program(JAVAC_EXECUTABLE javac)
find_program(JAVA_EXECUTABLE java)

if(JAVAC_EXECUTABLE AND JAVA_EXECUTABLE)
    execute_process(COMMAND ${JAVAC_EXECUTABLE} -version
        RESULT_VARIABLE javac_rc
        OUTPUT_VARIABLE javac_version ERROR_VARIABLE javac_version
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
endif()

if(NOT JAVAC_EXECUTABLE OR NOT JAVA_EXECUTABLE OR NOT javac_rc EQUAL 0)
    message("JAVA_TOOLCHAIN_NOT_FOUND: no JDK on PATH; skipping Java round-trip test")
    return()
endif()

message("Using ${JAVAC_EXECUTABLE} (${javac_version})")

string(REPLACE "," ";" SPECS "${SPECS}")

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR}/classes)

# The package comes from each spec; the test class is named by the lang
# file's test file pattern, {classname}Test.java.
set(test_classes "")
foreach(spec ${SPECS})
    file(READ ${spec} spec_json)
    string(JSON package GET "${spec_json}" stringValues namespace)
    string(JSON classname GET "${spec_json}" stringValues classname)
    list(APPEND test_classes ${package}.${classname}Test)

    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

file(GLOB java_files RELATIVE ${OUTDIR} ${OUTDIR}/*.java)

execute_process(
    COMMAND ${JAVAC_EXECUTABLE} -Xlint:all -Werror -d classes ${java_files}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "javac failed (${rc})")
endif()

foreach(test_class ${test_classes})
    execute_process(
        COMMAND ${JAVA_EXECUTABLE} -cp classes ${test_class}
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Java round-trip ${test_class} failed (${rc})")
    endif()
endforeach()
