#
#  JavaScript round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects Node.js 18 or later at test time. Without it, it prints the skip
#  marker matched by the test's SKIP_REGULAR_EXPRESSION. Otherwise it
#  generates the modules and their tests with wiregen, adds a package.json
#  that makes Node 18 and 20 treat the .js files as ES modules, checks every
#  file's syntax, and runs the generated tests (plus EXTRA_TESTS, hand-written
#  ones that import the generated modules) with node --test.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  Optional: EXTRA_TESTS, comma-separated
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

# A version too old clears NODE_EXECUTABLE with set(): unset() would expose
# find_program's cache entry again.
find_program(NODE_EXECUTABLE node)
if(NODE_EXECUTABLE)
    execute_process(COMMAND ${NODE_EXECUTABLE} -p "process.versions.node"
        RESULT_VARIABLE node_rc
        OUTPUT_VARIABLE node_version ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT node_rc EQUAL 0 OR NOT node_version MATCHES "^[0-9]+\\.")
        set(NODE_EXECUTABLE "")
    elseif(node_version VERSION_LESS 18)
        message("Node ${node_version} is older than 18, which the generated tests need")
        set(NODE_EXECUTABLE "")
    endif()
endif()

if(NOT NODE_EXECUTABLE)
    message("JAVASCRIPT_NOT_FOUND: no Node.js 18 or later on PATH; skipping JavaScript round-trip test")
    return()
endif()

message("Using ${NODE_EXECUTABLE} (Node ${node_version})")

string(REPLACE "," ";" SPECS "${SPECS}")
string(REPLACE "," ";" EXTRA_TESTS "${EXTRA_TESTS}")

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})
file(WRITE ${OUTDIR}/package.json "{ \"type\": \"module\" }\n")

foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

foreach(test ${EXTRA_TESTS})
    file(COPY ${test} DESTINATION ${OUTDIR})
endforeach()

file(GLOB sources RELATIVE ${OUTDIR} ${OUTDIR}/*.js)
foreach(source ${sources})
    execute_process(
        COMMAND ${NODE_EXECUTABLE} --check ${source}
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${source} is not valid JavaScript (${rc})")
    endif()
endforeach()

file(GLOB tests RELATIVE ${OUTDIR} ${OUTDIR}/*.test.js)
list(SORT tests)
execute_process(
    COMMAND ${NODE_EXECUTABLE} --test ${tests}
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "JavaScript round-trip failed (${rc})")
endif()
