#
#  Copyright (C) 2017 Geoffrey Atkinson
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

set(TARGETNAME skywire_unit_tests)

include_directories(
                ${PROJECT_BINARY_DIR}/skywireproto
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/lang/cpp
                ${PROJECT_SOURCE_DIR}/lang/c
                ${PROTOZERO_INC_DIR})

set(WIREGEN_CPP_FILES)

set(UNIT_TESTS
                bitfields_unit_test.cpp
                cgenerator_unit_test.cpp
                cppgenerator_unit_test.cpp
                csharpgenerator_unit_test.cpp
                cstream_unit_test.cpp
                endian_unit_test.cpp
                filewriter_unit_test.cpp
                generator_unit_test.cpp
                processor_unit_test.cpp
                proto3generator_unit_test.cpp
                rustgenerator_unit_test.cpp
                specification_unit_test.cpp
                stream_unit_test.cpp
                pz_stream_unit_test.cpp
                test_values_unit_test.cpp)

set(UNIT_TEST_JSON specification_unit_test.json)
set(INVALID_UNIT_TEST_JSON invalid_specification_unit_test.json)

add_executable(${TARGETNAME} main.cpp ${WIREGEN_CPP_FILES} ${UNIT_TESTS})

if(MSVC)
    set(PLATFORM_LINK_LIBS ws2_32)
endif(MSVC)

target_link_libraries(${TARGETNAME} skywire GTest::GTest GTest::Main ${PLATFORM_LINK_LIBS})

# Generator tests read the shipped lang files.
target_compile_definitions(${TARGETNAME} PRIVATE SKYWIRE_LANG_DIR="${PROJECT_SOURCE_DIR}/lang")

gtest_discover_tests(${TARGETNAME} TEST_PREFIX skywire_unit_test:)

add_dependencies(${TARGETNAME} wiregen)

add_custom_command(
    TARGET ${TARGETNAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/cfg/${UNIT_TEST_JSON}
        ${CMAKE_CURRENT_BINARY_DIR}/${UNIT_TEST_JSON})

add_custom_command(
    TARGET ${TARGETNAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${PROJECT_SOURCE_DIR}/cfg/${INVALID_UNIT_TEST_JSON}
        ${CMAKE_CURRENT_BINARY_DIR}/${INVALID_UNIT_TEST_JSON})



# How wiregen picks its lang file: --language finds the one next to wiregen,
# --lang_file (or the old --langfile) replaces it, and the two must agree.
set(CLI_SPEC ${PROJECT_SOURCE_DIR}/src/test/generated_tests/ClassyMcClassface.json)

add_test(NAME wiregen_cli:Language
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC})
add_test(NAME wiregen_cli:LanguageAlias
    COMMAND wiregen --language cpp --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:Language wiregen_cli:LanguageAlias PROPERTIES
    PASS_REGULAR_EXPRESSION "class ClassyMcClassface")

# Rust writes one module per message, named in snake_case.
add_test(NAME wiregen_cli:LanguageRust
    COMMAND wiregen --language rust --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:LanguageRust PROPERTIES
    PASS_REGULAR_EXPRESSION "pub struct ClassyMcClassface")

add_test(NAME wiregen_cli:LanguageCSharp
    COMMAND wiregen --language "c#" --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:LanguageCSharp PROPERTIES
    PASS_REGULAR_EXPRESSION "public sealed class ClassyMcClassface")

add_test(NAME wiregen_cli:LangFile
    COMMAND wiregen --lang_file ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json --input_file ${CLI_SPEC})
add_test(NAME wiregen_cli:LangfileOldSpelling
    COMMAND wiregen --langfile ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json --input_file ${CLI_SPEC})
add_test(NAME wiregen_cli:LangFileReplacesInstalled
    COMMAND wiregen --language c --lang_file ${PROJECT_SOURCE_DIR}/lang/c/lang.c.json --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:LangFile wiregen_cli:LangfileOldSpelling wiregen_cli:LangFileReplacesInstalled PROPERTIES
    PASS_REGULAR_EXPRESSION "extern \"C\"")

add_test(NAME wiregen_cli:LanguageMismatch
    COMMAND wiregen --language c --lang_file ${PROJECT_SOURCE_DIR}/lang/cpp/lang.cpp.json --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:LanguageMismatch PROPERTIES
    PASS_REGULAR_EXPRESSION "lang file is for c\\+\\+, not c")

add_test(NAME wiregen_cli:UnknownLanguage
    COMMAND wiregen --language cobol --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:UnknownLanguage PROPERTIES
    PASS_REGULAR_EXPRESSION "Unknown language: cobol")

add_test(NAME wiregen_cli:NoLanguage
    COMMAND wiregen --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:NoLanguage PROPERTIES
    PASS_REGULAR_EXPRESSION "Choose a language")

# --namespace replaces the spec's namespace.
add_test(NAME wiregen_cli:Namespace
    COMMAND wiregen --language c++ --namespace other --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:Namespace PROPERTIES
    PASS_REGULAR_EXPRESSION "namespace other")

# Generated code carries no license obligation (README "License of generated
# code"), so nothing wiregen writes, tests and relays included, may name a
# copyright holder or a license.
foreach(LANG c cpp csharp go java proto3 python3 rust swift)
    add_test(NAME wiregen_cli:NoLicenseText_${LANG}
        COMMAND wiregen --language ${LANG} --generate_tests --generate_relay --input_file ${CLI_SPEC})
    set_tests_properties(wiregen_cli:NoLicenseText_${LANG} PROPERTIES
        FAIL_REGULAR_EXPRESSION "Copyright|Licensed under|Apache|Skyloupe, LLC|Geoffrey Atkinson")
endforeach()

add_test(NAME wiregen_cli:Version
    COMMAND wiregen --version)
set_tests_properties(wiregen_cli:Version PROPERTIES
    PASS_REGULAR_EXPRESSION "^wiregen ${PROJECT_VERSION}\n$")

# --banner on its own prints the banner and succeeds; with a spec it prints
# the banner, then generates as usual.
add_test(NAME wiregen_cli:Banner
    COMMAND wiregen --banner)
set_tests_properties(wiregen_cli:Banner PROPERTIES
    PASS_REGULAR_EXPRESSION "^\\+-+\\+\n\\| wiregen ${PROJECT_VERSION}[ (]")
add_test(NAME wiregen_cli:BannerBuildInfo
    COMMAND wiregen --banner)
set_tests_properties(wiregen_cli:BannerBuildInfo PROPERTIES
    PASS_REGULAR_EXPRESSION "\\| compiler:  ${CMAKE_CXX_COMPILER_ID} [^\n]*\n\\| build:     [^\n]*${CMAKE_SYSTEM_NAME}")
add_test(NAME wiregen_cli:BannerThenGenerate
    COMMAND wiregen --banner --language c++ --namespace other --input_file ${CLI_SPEC})
set_tests_properties(wiregen_cli:BannerThenGenerate PROPERTIES
    PASS_REGULAR_EXPRESSION "\\| wiregen ${PROJECT_VERSION}.*namespace other")

add_test(NAME wiregen_cli:Help
    COMMAND wiregen --help)
set_tests_properties(wiregen_cli:Help PROPERTIES
    PASS_REGULAR_EXPRESSION "--namespace name")

# Every argument must mean something: a removed (--config) or misspelled
# option, a missing value, or a value given to a flag is an error.
add_test(NAME wiregen_cli:NoArguments
    COMMAND wiregen)
add_test(NAME wiregen_cli:UnknownOptionFails
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --config settings.json)
add_test(NAME wiregen_cli:MissingValueFails
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --outdir)
add_test(NAME wiregen_cli:FlagValueFails
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --silent yes)
set_tests_properties(wiregen_cli:NoArguments wiregen_cli:UnknownOptionFails
                     wiregen_cli:MissingValueFails wiregen_cli:FlagValueFails PROPERTIES
    WILL_FAIL TRUE)

add_test(NAME wiregen_cli:UnknownOption
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --config settings.json)
set_tests_properties(wiregen_cli:UnknownOption PROPERTIES
    PASS_REGULAR_EXPRESSION "Unknown option: --config")
add_test(NAME wiregen_cli:MissingValue
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --outdir)
set_tests_properties(wiregen_cli:MissingValue PROPERTIES
    PASS_REGULAR_EXPRESSION "--outdir needs a value")
add_test(NAME wiregen_cli:FlagValue
    COMMAND wiregen --language c++ --input_file ${CLI_SPEC} --silent yes)
set_tests_properties(wiregen_cli:FlagValue PROPERTIES
    PASS_REGULAR_EXPRESSION "--silent takes no value")
