#
#  Copyright (C) 2026 Geoffrey Atkinson
#
#  Licensed under the Apache License, Version 2.0 (the "License").
#  You may not use this file except in compliance with the License.
#  A copy of the License is located at
#
#  https://www.apache.org/licenses/LICENSE-2.0.html
#
#  or in the "license" file accompanying this file. This file is distributed
#  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
#  express or implied. See the License for the specific language governing
#  permissions and limitations under the License.
#
# skywire_text_diff:<language>: the language's generated JSON, XML and BSON
# code must accept exactly the inputs the C++ does, and write back exactly
# the same bytes. Generates the ring message in LANGUAGE, makes the inputs
# (make_inputs.py: the fuzz corpora and COUNT seeded mutations of them), and
# compares the language's driver with ORACLE (oracle.cpp).
#
#   cmake -DLANGUAGE=python3 -DWIREGEN=... -DORACLE=... -DPYTHON=... [-DGO=...] [-DDOTNET=...] [-DRUSTC=...] -DCOUNT=30000
#         -DSOURCE_DIR=src/test/text_diff -DRING_DIR=... -DCORPORA="a|b" -DOUTDIR=... -P text_diff.cmake

if(NOT PYTHON)
    message("TEXT_DIFF_SKIPPED: no Python 3, which makes the inputs and compares")
    return()
endif()
if(LANGUAGE STREQUAL "go" AND NOT GO)
    message("TEXT_DIFF_SKIPPED: no go")
    return()
endif()
if(LANGUAGE STREQUAL "rust" AND NOT RUSTC)
    message("TEXT_DIFF_SKIPPED: no rustc")
    return()
endif()
if(LANGUAGE STREQUAL "csharp")
    # Only the runtime, not an SDK, may be installed.
    if(DOTNET)
        execute_process(COMMAND ${DOTNET} --version RESULT_VARIABLE rc
            OUTPUT_VARIABLE dotnet_version ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(NOT rc EQUAL 0 OR NOT dotnet_version MATCHES "^[0-9]+\\.")
            set(DOTNET "")
        endif()
    endif()
    if(NOT DOTNET)
        message("TEXT_DIFF_SKIPPED: no .NET SDK")
        return()
    endif()
endif()

string(REPLACE "|" ";" CORPORA "${CORPORA}")
file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR}/generated)

function(run what)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${what} failed (${rc}):\n${out}")
    endif()
endfunction()

foreach(spec RingyInner RingyMcRingface)
    run("wiregen ${spec}" ${WIREGEN} --language ${LANGUAGE} --generate_json --generate_xml --generate_bson
        --silent --write_to_file --outdir ${OUTDIR}/generated --input_file ${RING_DIR}/${spec}.json)
endforeach()

set(inputs ${OUTDIR}/inputs.bin)
run("make_inputs.py" ${PYTHON} ${SOURCE_DIR}/make_inputs.py ${inputs} ${COUNT} ${CORPORA})
execute_process(COMMAND ${ORACLE} ${inputs} OUTPUT_FILE ${OUTDIR}/want.txt RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "the C++ oracle failed (${rc})")
endif()

if(LANGUAGE STREQUAL "python3")
    execute_process(COMMAND ${PYTHON} ${SOURCE_DIR}/driver.py ${OUTDIR}/generated ${inputs}
        OUTPUT_FILE ${OUTDIR}/got.txt RESULT_VARIABLE rc ERROR_VARIABLE err)
elseif(LANGUAGE STREQUAL "go")
    # A module holding the generated package and the driver, built with the
    # local toolchain only.
    set(ENV{GOTOOLCHAIN} local)
    file(WRITE ${OUTDIR}/go.mod "module textdiff\n\ngo 1.21\n")
    configure_file(${SOURCE_DIR}/driver.go ${OUTDIR}/driver/main.go COPYONLY)
    set(driver ${OUTDIR}/driver_go)
    if(CMAKE_HOST_WIN32)
        string(APPEND driver .exe)
    endif()
    run("go build" ${GO} build -o ${driver} ./driver WORKING_DIRECTORY ${OUTDIR})
    execute_process(COMMAND ${driver} ${inputs}
        OUTPUT_FILE ${OUTDIR}/got.txt RESULT_VARIABLE rc ERROR_VARIABLE err)
elseif(LANGUAGE STREQUAL "csharp")
    # A project holding the generated classes, SkywireText.cs and the
    # driver, built as the C# round-trip test builds: C# 7.3, warnings as
    # errors, overflow checked.
    set(ENV{DOTNET_CLI_TELEMETRY_OPTOUT} 1)
    set(ENV{DOTNET_NOLOGO} 1)
    set(ENV{DOTNET_SKIP_FIRST_TIME_EXPERIENCE} 1)
    string(REGEX MATCH "^[0-9]+" dotnet_major "${dotnet_version}")
    configure_file(${SOURCE_DIR}/Driver.cs ${OUTDIR}/Driver.cs COPYONLY)
    file(WRITE ${OUTDIR}/TextDiff.csproj
"<Project Sdk=\"Microsoft.NET.Sdk\">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net${dotnet_major}.0</TargetFramework>
    <LangVersion>7.3</LangVersion>
    <Nullable>disable</Nullable>
    <ImplicitUsings>disable</ImplicitUsings>
    <TreatWarningsAsErrors>true</TreatWarningsAsErrors>
    <CheckForOverflowUnderflow>true</CheckForOverflowUnderflow>
    <CheckEolTargetFramework>false</CheckEolTargetFramework>
  </PropertyGroup>
</Project>
")
    run("dotnet build" ${DOTNET} build -c Release -o ${OUTDIR}/out WORKING_DIRECTORY ${OUTDIR})
    execute_process(COMMAND ${DOTNET} ${OUTDIR}/out/TextDiff.dll ${inputs}
        OUTPUT_FILE ${OUTDIR}/got.txt RESULT_VARIABLE rc ERROR_VARIABLE err)
elseif(LANGUAGE STREQUAL "rust")
    # A crate whose root is the driver, the generated modules beside it.
    configure_file(${SOURCE_DIR}/driver.rs ${OUTDIR}/generated/main.rs COPYONLY)
    set(driver ${OUTDIR}/driver_rust)
    if(CMAKE_HOST_WIN32)
        string(APPEND driver .exe)
    endif()
    run("rustc" ${RUSTC} --edition 2021 -O -o ${driver} main.rs WORKING_DIRECTORY ${OUTDIR}/generated)
    execute_process(COMMAND ${driver} ${inputs}
        OUTPUT_FILE ${OUTDIR}/got.txt RESULT_VARIABLE rc ERROR_VARIABLE err)
else()
    message(FATAL_ERROR "text_diff.cmake: no driver for ${LANGUAGE}")
endif()
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "the ${LANGUAGE} driver failed (${rc}):\n${err}")
endif()

execute_process(COMMAND ${PYTHON} ${SOURCE_DIR}/compare.py ${inputs} ${OUTDIR}/want.txt ${OUTDIR}/got.txt ${LANGUAGE}
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
message("${out}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "${LANGUAGE} and C++ disagree")
endif()
