#
#  C# round-trip test driver, run by CTest via `cmake -P`.
#
#  Detects the .NET SDK (dotnet) at test time. Without it, it prints the skip
#  marker matched by the test's SKIP_REGULAR_EXPRESSION. Otherwise it
#  generates the C# classes and their tests with wiregen, builds them into one
#  program at C# 7.3 with warnings as errors and overflow checking on (so any
#  narrowing the generated code leaves out of unchecked() fails the test), and
#  runs every generated test's Run() from a Driver class: once with the Span
#  fast path, and once with SKYWIRE_PORTABLE defined, as .NET Framework and
#  Unity would build it. The classes have their JSON, XML and BSON forms,
#  and the tests check them, with SkywireText.cs beside them.
#
#  Required -D variables: WIREGEN, SPECS, LANGFILE, OUTDIR
#  SPECS is a comma-separated list (CTest splits arguments on ';'); a spec's
#  nested message types must be among them.
#

set(ENV{DOTNET_CLI_TELEMETRY_OPTOUT} 1)
set(ENV{DOTNET_NOLOGO} 1)
set(ENV{DOTNET_SKIP_FIRST_TIME_EXPERIENCE} 1)

find_program(DOTNET_EXECUTABLE dotnet)
if(DOTNET_EXECUTABLE)
    # Only the runtime, not an SDK, may be installed.
    execute_process(COMMAND ${DOTNET_EXECUTABLE} --version
        RESULT_VARIABLE dotnet_rc
        OUTPUT_VARIABLE dotnet_version ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT dotnet_rc EQUAL 0 OR NOT dotnet_version MATCHES "^[0-9]+\\.")
        set(DOTNET_EXECUTABLE "")
    endif()
endif()

if(NOT DOTNET_EXECUTABLE)
    message("CSHARP_TOOLCHAIN_NOT_FOUND: no .NET SDK on PATH; skipping C# round-trip test")
    return()
endif()

message("Using ${DOTNET_EXECUTABLE} (SDK ${dotnet_version})")

string(REPLACE "," ";" SPECS "${SPECS}")

file(REMOVE_RECURSE ${OUTDIR})
file(MAKE_DIRECTORY ${OUTDIR})

# Each test class is {namespace}.{classname}Test, by the lang file's test
# file pattern, {classname}Test.cs.
set(run_calls "")
foreach(spec ${SPECS})
    file(READ ${spec} spec_json)
    string(JSON namespace GET "${spec_json}" stringValues namespace)
    string(JSON classname GET "${spec_json}" stringValues classname)
    string(APPEND run_calls "        failures += ${namespace}.${classname}Test.Run();\n")

    execute_process(
        COMMAND ${WIREGEN} --input_file ${spec} --outdir ${OUTDIR}
                --lang_file ${LANGFILE} --generate_tests --write_to_file
                --generate_json --generate_xml --generate_bson
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "wiregen failed on ${spec} (${rc})")
    endif()
endforeach()

file(WRITE ${OUTDIR}/Driver.cs
"public static class Driver
{
    public static int Main()
    {
        int failures = 0;
${run_calls}        return failures == 0 ? 0 : 1;
    }
}
")

string(REGEX MATCH "^[0-9]+" dotnet_major "${dotnet_version}")

file(WRITE ${OUTDIR}/CSharpRoundTrip.csproj
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
    <DefineConstants>$(DefineConstants);SKYWIRE_NO_MAIN;$(SkywireConstants)</DefineConstants>
    <StartupObject>Driver</StartupObject>
  </PropertyGroup>
</Project>
")

# MSBuild takes environment variables as properties.
foreach(constants "" "SKYWIRE_PORTABLE")
    message("Building with SkywireConstants='${constants}'")
    set(ENV{SkywireConstants} "${constants}")
    execute_process(
        COMMAND ${DOTNET_EXECUTABLE} run -c Release
        WORKING_DIRECTORY ${OUTDIR}
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "C# round-trip failed with SkywireConstants='${constants}' (${rc})")
    endif()
endforeach()
