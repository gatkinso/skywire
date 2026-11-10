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
# Golden files: what wiregen writes for every language with no optional
# output (--generate_json, --generate_xml, ...), so a change to a generator
# cannot quietly change the code users already have. ClassyMcClassface has a
# member of every type; Telemetry has bitfields, a string and a nested
# message (its relay reads Position.json beside it).
#
# After a deliberate change to generated code, build skywire_update_golden
# and commit the new files.

set(GOLDEN_SPECS
    ${PROJECT_SOURCE_DIR}/src/test/generated_tests/ClassyMcClassface.json
    ${PROJECT_SOURCE_DIR}/examples/specs/Telemetry.json)
set(GOLDEN_LANGUAGES c cpp csharp go java javascript proto3 python3 rust swift)

# Specs are joined with | so the list survives being one argument.
string(REPLACE ";" "|" GOLDEN_SPEC_ARG "${GOLDEN_SPECS}")

set(UPDATE_COMMANDS)
foreach(LANG IN LISTS GOLDEN_LANGUAGES)
    set(ARGS -DWIREGEN=$<TARGET_FILE:wiregen> -DLANGUAGE=${LANG}
             -DSPECS=${GOLDEN_SPEC_ARG}
             -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/${LANG}
             -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/${LANG})
    add_test(NAME skywire_golden:${LANG}
        COMMAND ${CMAKE_COMMAND} ${ARGS} -P ${CMAKE_CURRENT_SOURCE_DIR}/golden.cmake)
    list(APPEND UPDATE_COMMANDS
        COMMAND ${CMAKE_COMMAND} ${ARGS} -DUPDATE=ON -P ${CMAKE_CURRENT_SOURCE_DIR}/golden.cmake)
endforeach()

add_custom_target(skywire_update_golden
    ${UPDATE_COMMANDS}
    DEPENDS wiregen
    COMMENT "Replacing src/test/golden with wiregen's current output"
    VERBATIM)
