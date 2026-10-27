#
#  TypeScript declarations test driver, run by CTest via `cmake -P`.
#
#  Detects tsc 5.1 or later at test time (never npx, which would need the
#  network). Without it, it prints the skip marker matched by the test's
#  SKIP_REGULAR_EXPRESSION. Otherwise it generates the modules and their
#  .d.ts files with wiregen, writes usage.ts, which reads and writes every
#  member, calls every method, and expects an error from each wrongly typed
#  assignment (@ts-expect-error), and type-checks it with --strict.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

# On Windows npm installs a tsc.cmd beside a tsc shell script for Git Bash,
# which CreateProcess can't run.
find_program(TSC_EXECUTABLE NAMES tsc.cmd tsc)
if(TSC_EXECUTABLE)
    execute_process(COMMAND ${TSC_EXECUTABLE} --version
        RESULT_VARIABLE tsc_rc
        OUTPUT_VARIABLE tsc_version ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+\\.[0-9]+(\\.[0-9]+)?" tsc_version "${tsc_version}")
    if(NOT tsc_rc EQUAL 0 OR NOT tsc_version)
        set(TSC_EXECUTABLE "")
    elseif(tsc_version VERSION_LESS 5.1)
        message("tsc ${tsc_version} is older than 5.1, which the declarations need")
        set(TSC_EXECUTABLE "")
    endif()
endif()

if(NOT TSC_EXECUTABLE)
    message("TYPESCRIPT_NOT_FOUND: no TypeScript 5.1 or later on PATH; skipping JavaScript types test")
    return()
endif()

message("Using ${TSC_EXECUTABLE} (TypeScript ${tsc_version})")

string(REPLACE "," ";" SPECS "${SPECS}")

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})
file(WRITE ${OUTDIR}/package.json "{ \"type\": \"module\" }\n")

set(imports "")
set(body "")
foreach(spec ${SPECS})
    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --write_to_file
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()

    file(READ ${spec} spec_json)
    string(JSON classname GET "${spec_json}" stringValues classname)
    string(APPEND imports "import { ${classname} } from \"./${classname}.js\";\n")

    set(m "m_${classname}")
    string(APPEND body
        "\nconst ${m}: ${classname} = new ${classname}();\n"
        "const ${m}_size: number = ${classname}.SIZE + ${m}.size();\n"
        "const ${m}_wire: Uint8Array = ${m}.serialize();\n"
        "const ${m}_end: number = ${m}.serializeInto(new DataView(new ArrayBuffer(${classname}.SIZE)), 0)\n"
        "  + ${m}.serializeInto(new Uint8Array(${classname}.SIZE)) + ${m}.serializeInto(new ArrayBuffer(${classname}.SIZE));\n"
        "${m}.deserialize(${m}_wire);\n"
        "const ${m}_next: number = ${m}.deserializeFrom(${m}_wire, 0) + ${m}.deserializeFrom(new DataView(${m}_wire.buffer));\n"
        "const ${m}_copy: ${classname} = ${classname}.fromBytes(${m}_wire.buffer);\n"
        "const ${m}_same: boolean = ${m}.equals(${m}_copy) && ${m}.equals(null);\n"
        "const ${m}_text: string = ${m}.toString() + JSON.stringify(${m}.toJSON());\n"
        "void [${m}_size, ${m}_end, ${m}_next, ${m}_same, ${m}_text];\n"
        "// @ts-expect-error SIZE is read-only\n"
        "${classname}.SIZE = 1;\n")

    string(JSON members GET "${spec_json}" members)
    string(JSON count LENGTH "${members}")
    math(EXPR last "${count} - 1")
    foreach(i RANGE ${last})
        string(JSON tag MEMBER "${members}" ${i})
        string(JSON type GET "${members}" ${tag} type)
        string(JSON name GET "${members}" ${tag} name)
        set(p "${m}.${name}")
        if(type STREQUAL "string")
            string(APPEND body
                "${p} = \"text\";\n${p} = new Uint8Array(2);\n${p} = [1, 2];\n"
                "const ${m}_${name}: Uint8Array = ${p};\n"
                "const ${m}_${name}Text: string = ${p}Text;\n"
                "void [${m}_${name}, ${m}_${name}Text];\n"
                "// @ts-expect-error a string member takes text or bytes\n${p} = 5;\n"
                "// @ts-expect-error the text getter is read-only\n${p}Text = \"x\";\n")
        elseif(type MATCHES "^u?int64_t$")
            string(APPEND body
                "${p} = ${p} + 1n;\n${p} = 7;\n"
                "// @ts-expect-error a 64-bit member reads as a bigint\n"
                "const ${m}_${name}: number = ${p};\n"
                "void ${m}_${name};\n"
                "// @ts-expect-error a 64-bit member takes a bigint or a number\n${p} = \"7\";\n")
        elseif(type MATCHES "^(u?int(8|16|32)_t|char|unsigned char|float|double)$")
            string(APPEND body
                "${p} = ${p} + 1;\n"
                "// @ts-expect-error a ${type} member reads as a number\n"
                "const ${m}_${name}: string = ${p};\n"
                "void ${m}_${name};\n"
                "// @ts-expect-error a ${type} member takes a number\n${p} = 7n;\n")
        else()
            # A nested message: its members are reached through it.
            string(APPEND body
                "${p} = new ${type}();\n"
                "const ${m}_${name}: ${type} = ${p};\n"
                "void ${m}_${name};\n"
                "// @ts-expect-error a nested message member takes its type\n${p} = {};\n")
        endif()
    endforeach()
endforeach()

file(WRITE ${OUTDIR}/usage.ts "// Generated by js_types.cmake.\n\n${imports}${body}")

# No @types packages: the declarations must stand on their own.
file(WRITE ${OUTDIR}/tsconfig.json
"{
  \"compilerOptions\": {
    \"target\": \"es2020\",
    \"lib\": [\"es2020\"],
    \"module\": \"nodenext\",
    \"moduleResolution\": \"nodenext\",
    \"strict\": true,
    \"noEmit\": true,
    \"types\": [],
    \"skipLibCheck\": false
  },
  \"files\": [\"usage.ts\"]
}
")

execute_process(
    COMMAND ${TSC_EXECUTABLE} -p tsconfig.json
    WORKING_DIRECTORY ${OUTDIR}
    RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "the generated declarations do not type-check (${rc})")
endif()
