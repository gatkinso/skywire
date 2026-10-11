#
#  Java round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects a JDK at test time. If javac is absent (or is the macOS stub that
#  only prompts to install one) it prints the skip marker matched by the test's
#  SKIP_REGULAR_EXPRESSION; otherwise it generates the Java sources with
#  wiregen, compiles them with all lint warnings as errors, and runs the
#  generated test class.
#
#  Required -D variables: WIREGEN, SPEC, LANGFILE, OUTDIR
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

# The package comes from the spec; the test class is named by the lang
# file's test file pattern, {classname}Test.java.
file(READ ${SPEC} spec_json)
string(JSON package GET "${spec_json}" stringValues namespace)
string(JSON classname GET "${spec_json}" stringValues classname)
set(test_class ${classname}Test)

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR}/classes)

execute_process(
    COMMAND ${WIREGEN} --input_file ${SPEC} --outdir ${OUTDIR}
            --lang_file ${LANGFILE} --generate_tests --write_to_file
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "wiregen failed (${rc})")
endif()

file(GLOB java_files RELATIVE ${OUTDIR} ${OUTDIR}/*.java)

execute_process(
    COMMAND ${JAVAC_EXECUTABLE} -Xlint:all -Werror -d classes ${java_files}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "javac failed (${rc})")
endif()

execute_process(
    COMMAND ${JAVA_EXECUTABLE} -cp classes ${package}.${test_class}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Java round-trip failed (${rc})")
endif()
