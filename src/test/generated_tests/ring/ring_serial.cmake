#
#  Cross-language serial test driver, run by CTest via `cmake -P`.
#
#  Passes the checked-in serialized message (RingyMcRingface.bin) through the
#  generated relay of every available language, one after another, in a random
#  order:
#
#      input.bin -> L1 -> hop1.bin -> L2 -> hop2.bin -> ... -> Ln -> hopN.bin -> L1
#
#  The ring closes: the first language's relay (the same build) reads the last
#  hop's output, so the bytes that come back must satisfy the language they
#  started in.
#
#  Each relay deserializes its input file, checks every member against the
#  shared test values, re-serializes, checks the bytes are unchanged and writes
#  the next file. The driver also checks that every hop file is byte-for-byte
#  identical to the original input, so each hop starts from verified input and
#  a failure belongs to the language at that hop.
#
#  Before the chain runs, each relay must reject a damaged copy of the input
#  (one byte inverted, one byte short, one byte extra), which proves its checks
#  can fail.
#
#  Environment:
#    SKYWIRE_RING_SEED=<n>          repeat the order (and flipped byte) of an earlier run
#    SKYWIRE_RING_ORDER=cpp,c,...   run exactly this order (languages: c cpp swift go java python rust javascript)
#    SKYWIRE_RING_REQUIRE_ALL=1     fail, instead of skipping, a language whose toolchain is missing
#
#  Required -D variables: WIREGEN, INPUT, SPEC, INNER_SPEC, LANG_DIR, WORKDIR,
#                         C_RELAY, CPP_RELAY, TAMPER
#  Optional -D variables: EXE_SUFFIX
#

# Script mode starts with every policy at OLD; match the project's minimum so
# IN_LIST and the other newer features behave the same on every CMake version.
cmake_minimum_required(VERSION 3.28)

set(all_langs cpp c java swift go python rust javascript)

set(display_cpp "C++")
set(display_c "C")
set(display_java "Java")
set(display_swift "Swift")
set(display_go "Go")
set(display_python "Python")
set(display_rust "Rust")
set(display_javascript "JavaScript")

set(available "")

function(skip_lang lang reason)
    if("$ENV{SKYWIRE_RING_REQUIRE_ALL}")
        message(FATAL_ERROR "${display_${lang}}: ${reason} (SKYWIRE_RING_REQUIRE_ALL is set)")
    endif()
    message("RING_LANG_SKIPPED: ${lang}: ${reason}")
endfunction()

function(run_checked what)
    execute_process(COMMAND ${ARGN}
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc}):\n${out}")
    endif()
endfunction()

# Generates one language's code with the lang file installed next to wiregen:
# the ring message with its relay, and the message it contains (INNER_SPEC).
# Any further arguments (e.g. --namespace) are passed to wiregen.
function(generate lang)
    file(REMOVE_RECURSE ${WORKDIR}/${lang})
    file(MAKE_DIRECTORY ${WORKDIR}/${lang})
    run_checked("wiregen (${lang}, inner message)"
        ${WIREGEN} --input_file ${INNER_SPEC} --outdir ${WORKDIR}/${lang}
                   --language ${lang} --write_to_file ${ARGN})
    run_checked("wiregen (${lang})"
        ${WIREGEN} --input_file ${SPEC} --outdir ${WORKDIR}/${lang}
                   --language ${lang} --generate_relay --write_to_file ${ARGN})
endfunction()

file(REMOVE_RECURSE ${WORKDIR})
file(MAKE_DIRECTORY ${WORKDIR})

# --- Build a relay for each language whose toolchain is present ------------

# C and C++ relays are built with the project.
set(relay_c ${C_RELAY})
set(relay_cpp ${CPP_RELAY})
list(APPEND available c cpp)

# Swift
find_program(SWIFTC_EXECUTABLE swiftc)
if(SWIFTC_EXECUTABLE)
    generate(swift)
    set(dir ${WORKDIR}/swift)
    run_checked("swiftc"
        ${SWIFTC_EXECUTABLE} -o ${dir}/ring_relay${EXE_SUFFIX}
        ${LANG_DIR}/swift/SWStream.swift ${dir}/RingyInner.swift ${dir}/RingyMcRingface.swift ${dir}/RingyMcRingfaceRelay.swift)
    set(relay_swift ${dir}/ring_relay${EXE_SUFFIX})
    list(APPEND available swift)
else()
    skip_lang(swift "swiftc not on PATH")
endif()

# Go (1.21 added binary.NativeEndian, which the generated code uses)
find_program(GO_EXECUTABLE go)
if(GO_EXECUTABLE)
    set(ENV{GOTOOLCHAIN} local)
    execute_process(COMMAND ${GO_EXECUTABLE} env GOVERSION
        OUTPUT_VARIABLE go_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^go([0-9]+\\.[0-9]+).*" "\\1" go_version_num "${go_version}")
endif()
if(GO_EXECUTABLE AND go_version_num VERSION_GREATER_EQUAL 1.21)
    # The Go relay is a command, so the generated code is in package main.
    generate(go --namespace main)
    set(dir ${WORKDIR}/go)
    file(WRITE ${dir}/go.mod "module wiregen/ring\n\ngo 1.21\n")
    get_filename_component(go_bindir ${GO_EXECUTABLE} DIRECTORY)
    find_program(GOFMT_EXECUTABLE gofmt HINTS ${go_bindir})
    if(GOFMT_EXECUTABLE)
        execute_process(COMMAND ${GOFMT_EXECUTABLE} -l .
            WORKING_DIRECTORY ${dir}
            OUTPUT_VARIABLE unformatted OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(unformatted)
            message(FATAL_ERROR "generated Go is not gofmt-clean: ${unformatted}")
        endif()
    endif()
    execute_process(COMMAND ${GO_EXECUTABLE} vet .
        WORKING_DIRECTORY ${dir} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "go vet failed (${rc}):\n${out}")
    endif()
    execute_process(COMMAND ${GO_EXECUTABLE} build -o ring_relay${EXE_SUFFIX} .
        WORKING_DIRECTORY ${dir} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "go build failed (${rc}):\n${out}")
    endif()
    set(relay_go ${dir}/ring_relay${EXE_SUFFIX})
    list(APPEND available go)
elseif(GO_EXECUTABLE)
    skip_lang(go "${go_version} is older than go1.21")
else()
    skip_lang(go "go not on PATH")
endif()

# Java (javac -version also rejects the macOS stub that has no JDK behind it)
find_program(JAVAC_EXECUTABLE javac)
find_program(JAVA_EXECUTABLE java)
if(JAVAC_EXECUTABLE AND JAVA_EXECUTABLE)
    execute_process(COMMAND ${JAVAC_EXECUTABLE} -version
        RESULT_VARIABLE javac_rc OUTPUT_QUIET ERROR_QUIET)
endif()
if(JAVAC_EXECUTABLE AND JAVA_EXECUTABLE AND javac_rc EQUAL 0)
    generate(java)
    set(dir ${WORKDIR}/java)
    file(MAKE_DIRECTORY ${dir}/classes)
    run_checked("javac"
        ${JAVAC_EXECUTABLE} -Xlint:all -Werror -d ${dir}/classes
        ${dir}/RingyInner.java ${dir}/RingyMcRingface.java ${dir}/RingyMcRingfaceRelay.java)
    set(relay_java ${JAVA_EXECUTABLE} -cp ${dir}/classes ring.RingyMcRingfaceRelay)
    list(APPEND available java)
else()
    skip_lang(java "no JDK on PATH")
endif()

# Python 3
find_program(PYTHON3_EXECUTABLE NAMES python3 python)
if(PYTHON3_EXECUTABLE)
    execute_process(COMMAND ${PYTHON3_EXECUTABLE} -c "import sys; sys.exit(sys.version_info[0] != 3)"
        RESULT_VARIABLE py_rc OUTPUT_QUIET ERROR_QUIET)
endif()
if(PYTHON3_EXECUTABLE AND py_rc EQUAL 0)
    generate(python)
    set(relay_python ${PYTHON3_EXECUTABLE} ${WORKDIR}/python/RingyMcRingfaceRelay.py)
    list(APPEND available python)
else()
    skip_lang(python "python3 not on PATH")
endif()

# Rust (1.58 added the inline format arguments the relay uses). rustup
# installs into ~/.cargo/bin, which a non-interactive shell may not have on
# PATH. The relay is the crate root and declares the message modules.
set(cargo_bins $ENV{HOME}/.cargo/bin $ENV{USERPROFILE}/.cargo/bin)
find_program(RUSTC_EXECUTABLE rustc HINTS ${cargo_bins})
if(RUSTC_EXECUTABLE)
    execute_process(COMMAND ${RUSTC_EXECUTABLE} --version
        OUTPUT_VARIABLE rustc_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^rustc ([0-9]+\\.[0-9]+).*" "\\1" rustc_version_num "${rustc_version}")
endif()
if(RUSTC_EXECUTABLE AND rustc_version_num VERSION_GREATER_EQUAL 1.58)
    generate(rust)
    set(dir ${WORKDIR}/rust)
    get_filename_component(rust_bindir ${RUSTC_EXECUTABLE} DIRECTORY)
    find_program(RUSTFMT_EXECUTABLE rustfmt HINTS ${rust_bindir} ${cargo_bins})
    if(RUSTFMT_EXECUTABLE)
        file(GLOB rust_sources ${dir}/*.rs)
        execute_process(COMMAND ${RUSTFMT_EXECUTABLE} --edition 2021 --check ${rust_sources}
            RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "generated Rust is not rustfmt-clean:\n${out}")
        endif()
    endif()
    run_checked("rustc"
        ${RUSTC_EXECUTABLE} --edition 2021 -D warnings -O -o ${dir}/ring_relay${EXE_SUFFIX}
        ${dir}/ringy_mc_ringface_relay.rs)
    find_program(CLIPPY_EXECUTABLE clippy-driver HINTS ${rust_bindir} ${cargo_bins})
    if(CLIPPY_EXECUTABLE)
        run_checked("clippy"
            ${CLIPPY_EXECUTABLE} --edition 2021 -D warnings -o ${dir}/clippy_relay${EXE_SUFFIX}
            ${dir}/ringy_mc_ringface_relay.rs)
    endif()
    set(relay_rust ${dir}/ring_relay${EXE_SUFFIX})
    list(APPEND available rust)
elseif(RUSTC_EXECUTABLE)
    skip_lang(rust "${rustc_version} is older than rustc 1.58")
else()
    skip_lang(rust "rustc not on PATH or in ~/.cargo/bin")
endif()

# JavaScript: Node 18 or later. The package.json makes Node 18 and 20 treat
# the generated .js files as ES modules.
find_program(NODE_EXECUTABLE node)
if(NODE_EXECUTABLE)
    execute_process(COMMAND ${NODE_EXECUTABLE} -p "process.versions.node"
        RESULT_VARIABLE node_rc OUTPUT_VARIABLE node_version ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(NODE_EXECUTABLE AND node_rc EQUAL 0 AND node_version VERSION_GREATER_EQUAL 18)
    generate(javascript)
    file(WRITE ${WORKDIR}/javascript/package.json "{ \"type\": \"module\" }\n")
    set(relay_javascript ${NODE_EXECUTABLE} ${WORKDIR}/javascript/RingyMcRingfaceRelay.js)
    list(APPEND available javascript)
elseif(NODE_EXECUTABLE)
    skip_lang(javascript "node ${node_version} is older than 18")
else()
    skip_lang(javascript "node not on PATH")
endif()

# --- Choose the order --------------------------------------------------------

if(NOT "$ENV{SKYWIRE_RING_SEED}" STREQUAL "")
    set(seed "$ENV{SKYWIRE_RING_SEED}")
else()
    string(RANDOM LENGTH 9 ALPHABET 123456789 seed)
endif()
if(NOT seed MATCHES "^[0-9]+$")
    message(FATAL_ERROR "SKYWIRE_RING_SEED must be a decimal number, not '${seed}'")
endif()
string(RANDOM LENGTH 1 ALPHABET 0 RANDOM_SEED ${seed} unused)

# Returns a pseudo-random integer in [0, bound) from the seeded generator.
function(random_below bound out)
    string(RANDOM LENGTH 6 ALPHABET 123456789 r)
    math(EXPR r "${r} % ${bound}")
    set(${out} ${r} PARENT_SCOPE)
endfunction()

if(NOT "$ENV{SKYWIRE_RING_ORDER}" STREQUAL "")
    string(REPLACE "," ";" order "$ENV{SKYWIRE_RING_ORDER}")
    foreach(lang ${order})
        if(NOT lang IN_LIST all_langs)
            message(FATAL_ERROR "SKYWIRE_RING_ORDER: unknown language '${lang}' (use: ${all_langs})")
        endif()
        if(NOT lang IN_LIST available)
            message(FATAL_ERROR "SKYWIRE_RING_ORDER: ${lang} is not available on this machine")
        endif()
    endforeach()
else()
    # Fisher-Yates shuffle of the available languages, in the canonical order first.
    set(order "")
    foreach(lang ${all_langs})
        if(lang IN_LIST available)
            list(APPEND order ${lang})
        endif()
    endforeach()
    list(LENGTH order n)
    math(EXPR i "${n} - 1")
    while(i GREATER 0)
        math(EXPR bound "${i} + 1")
        random_below(${bound} j)
        list(GET order ${i} a)
        list(GET order ${j} b)
        list(REMOVE_AT order ${i})
        list(INSERT order ${i} ${b})
        list(REMOVE_AT order ${j})
        list(INSERT order ${j} ${a})
        math(EXPR i "${i} - 1")
    endwhile()
endif()

list(LENGTH order n)
if(n LESS 2)
    message("RING_NOT_ENOUGH_LANGS: only '${order}' available; the serial test needs two languages")
    return()
endif()

# The first language closes the ring.
list(GET order 0 first_lang)
set(hops ${order} ${first_lang})

set(path "")
foreach(lang ${hops})
    list(APPEND path "${display_${lang}}")
endforeach()
string(REPLACE ";" " -> " path "${path}")
message("Order: ${path}")
message("Seed:  ${seed}  (repeat with SKYWIRE_RING_SEED=${seed})")

# --- Every relay must reject damaged input -------------------------------------

file(READ ${INPUT} input_hex HEX)
string(LENGTH "${input_hex}" hex_len)
math(EXPR input_size "${hex_len} / 2")
random_below(${input_size} flip_offset)

run_checked("ring_tamper flip" ${TAMPER} flip ${INPUT} ${WORKDIR}/flipped.bin ${flip_offset})
run_checked("ring_tamper truncate" ${TAMPER} truncate ${INPUT} ${WORKDIR}/short.bin)
run_checked("ring_tamper extend" ${TAMPER} extend ${INPUT} ${WORKDIR}/long.bin)

foreach(lang ${order})
    foreach(damage "flipped:byte ${flip_offset} inverted" "short:one byte short" "long:one byte extra")
        string(REPLACE ":" ";" damage "${damage}")
        list(GET damage 0 name)
        list(GET damage 1 description)
        execute_process(COMMAND ${relay_${lang}} ${WORKDIR}/${name}.bin ${WORKDIR}/${lang}_${name}_out.bin
            RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
        if(rc EQUAL 0)
            message(FATAL_ERROR "${display_${lang}} relay accepted damaged input (${description}):\n${out}")
        endif()
    endforeach()
endforeach()
message("Damaged input (byte ${flip_offset} inverted, one byte short, one byte extra) rejected by every relay")

# --- Relay the message through every language ----------------------------------

# Prints the first byte that differs between two hex strings.
function(describe_diff got_hex want_hex out)
    string(LENGTH "${got_hex}" got_len)
    string(LENGTH "${want_hex}" want_len)
    math(EXPR got_size "${got_len} / 2")
    math(EXPR want_size "${want_len} / 2")
    set(offset 0)
    while(offset LESS got_size AND offset LESS want_size)
        math(EXPR pos "${offset} * 2")
        string(SUBSTRING "${got_hex}" ${pos} 2 g)
        string(SUBSTRING "${want_hex}" ${pos} 2 w)
        if(NOT g STREQUAL w)
            break()
        endif()
        math(EXPR offset "${offset} + 1")
    endwhile()
    set(${out} "${got_size} bytes, want ${want_size}; first difference at byte ${offset}\n  got  ${got_hex}\n  want ${want_hex}" PARENT_SCOPE)
endfunction()

set(current ${INPUT})
set(previous "the input file")
set(hop 0)
foreach(lang ${hops})
    math(EXPR hop "${hop} + 1")
    set(next ${WORKDIR}/hop${hop}_${lang}.bin)

    execute_process(COMMAND ${relay_${lang}} ${current} ${next}
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    string(STRIP "${out}" out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "hop ${hop}: ${display_${lang}} failed on verified input from ${previous}:\n${out}")
    endif()
    if(NOT EXISTS ${next})
        message(FATAL_ERROR "hop ${hop}: ${display_${lang}} exited 0 but wrote no output\n${out}")
    endif()

    file(READ ${next} next_hex HEX)
    if(NOT next_hex STREQUAL input_hex)
        describe_diff("${next_hex}" "${input_hex}" diff)
        message(FATAL_ERROR "hop ${hop}: ${display_${lang}} wrote ${diff}\n${out}")
    endif()

    message("hop ${hop}: ${out}")
    set(current ${next})
    set(previous ${display_${lang}})
endforeach()

message("Serial test passed: ${path}")
