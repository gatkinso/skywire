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

add_subdirectory(lib)
add_subdirectory(bin)
if(BUILD_TESTING)
    add_subdirectory(test/skywire_unit_tests)
    add_subdirectory(test/generated_tests)
    add_subdirectory(test/golden)
    add_subdirectory(test/install)
endif()
# The fuzz targets need no GoogleTest; a fuzzing build may skip the tests.
if(BUILD_TESTING OR SKYWIRE_FUZZ)
    add_subdirectory(test/fuzz)
endif()
