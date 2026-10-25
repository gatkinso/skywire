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

#  Replays a libFuzzer target's seeds (SKYWIRE_FUZZ builds; other builds
#  replay with replay_main.cpp). libFuzzer runs each input once when every
#  argument is a file, but treats directories as corpora to fuzz, so the
#  directories in SEEDS ("|"-separated) are expanded here.
#
#    cmake -DFUZZER=<target> -DSEEDS=<file|dir|...> -P replay_corpus.cmake

string(REPLACE "|" ";" seeds "${SEEDS}")
set(inputs)
foreach(seed IN LISTS seeds)
    if(IS_DIRECTORY ${seed})
        file(GLOB files LIST_DIRECTORIES false ${seed}/*)
        list(SORT files)
        list(APPEND inputs ${files})
    else()
        list(APPEND inputs ${seed})
    endif()
endforeach()

if(NOT inputs)
    message(FATAL_ERROR "no inputs in ${SEEDS}")
endif()

execute_process(COMMAND ${FUZZER} ${inputs} RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "${FUZZER} failed on its corpus (${result})")
endif()
list(LENGTH inputs count)
message(STATUS "${count} inputs passed")
