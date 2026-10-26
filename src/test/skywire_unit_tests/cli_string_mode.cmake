#
#  wiregen string-mode test, run by CTest via `cmake -P`.
#
#  Passes a message spec with --input_str, and a lang file with --langspec,
#  as JSON on the command line. execute_process quotes the JSON for the
#  platform's shell, which add_test can't be relied on to do.
#
#  Required -D variables: WIREGEN, SPEC, LANGFILE, MODE
#  MODE is "language" (--language c++ --input_str), "langspec"
#  (--langspec <LANGFILE contents> --input_str) or "file_langspec"
#  (--langspec <LANGFILE contents> --input_file SPEC).
#

file(READ ${SPEC} spec_json)
set(input --input_str)
set(input_value "${spec_json}")
set(summary "Successful process: --input_str")

# The JSON goes straight into COMMAND, quoted: in a list variable its
# semicolons would split it into several arguments.
if(MODE STREQUAL "language")
    set(option --language)
    set(value c++)
    set(expected "class ClassyMcClassface")
elseif(MODE STREQUAL "langspec")
    set(option --langspec)
    file(READ ${LANGFILE} value)
    set(expected "extern \"C\"")
elseif(MODE STREQUAL "file_langspec")
    set(option --langspec)
    file(READ ${LANGFILE} value)
    set(expected "extern \"C\"")
    set(input --input_file)
    set(input_value ${SPEC})
    set(summary "Successful process: ${SPEC}")
else()
    message(FATAL_ERROR "Unknown MODE: ${MODE}")
endif()

execute_process(COMMAND ${WIREGEN} ${option} "${value}" ${input} "${input_value}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
    message(FATAL_ERROR "wiregen exited with ${rc}\n${out}${err}")
endif()
string(FIND "${out}" "${expected}" found)
if(found EQUAL -1)
    message(FATAL_ERROR "wiregen output lacks '${expected}':\n${out}${err}")
endif()
string(FIND "${out}" "${summary}" found)
if(found EQUAL -1)
    message(FATAL_ERROR "wiregen output lacks '${summary}':\n${out}${err}")
endif()
message("wiregen generated code (${MODE})")
