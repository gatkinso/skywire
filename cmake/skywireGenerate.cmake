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
#  skywire_generate(LANGUAGE <c|cpp>
#                   SPECS <spec.json>...
#                   [TARGET <target>]
#                   [OUT_DIR <dir>]
#                   [OUT_VAR <variable>]
#                   [NAMESPACE <namespace>]
#                   [JSON] [XML] [BSON])
#
#  Runs skywire::wiregen on each specification at build time. The files are
#  written to OUT_DIR (default ${CMAKE_CURRENT_BINARY_DIR}/skywire_<lang>) and
#  named as wiregen names them: the spec's outputheaderfilename and
#  outputimplfilename if it has them, otherwise {classname}.h (and .c for C).
#
#  With TARGET, the generated sources are added to the target, OUT_DIR is added
#  to its include path, and it links skywire::c_runtime or skywire::cpp_runtime.
#  OUT_VAR receives the generated files. A spec that nests another message
#  should be given with it, in the same directory, so a change to either
#  regenerates both. JSON, XML and BSON add those forms (wiregen
#  --generate_json, --generate_xml, --generate_bson; C++ only for now).

function(skywire_generate)
    cmake_parse_arguments(PARSE_ARGV 0 arg "JSON;XML;BSON" "LANGUAGE;TARGET;OUT_DIR;OUT_VAR;NAMESPACE" "SPECS")

    if(arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "skywire_generate: unknown arguments: ${arg_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT arg_SPECS)
        message(FATAL_ERROR "skywire_generate: SPECS is required")
    endif()

    if(arg_LANGUAGE STREQUAL "c")
        set(lang c)
    elseif(arg_LANGUAGE STREQUAL "cpp" OR arg_LANGUAGE STREQUAL "c++")
        set(lang cpp)
    else()
        message(FATAL_ERROR "skywire_generate: LANGUAGE must be c or cpp, not '${arg_LANGUAGE}'")
    endif()

    if(arg_OUT_DIR)
        get_filename_component(out_dir "${arg_OUT_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    else()
        set(out_dir "${CMAKE_CURRENT_BINARY_DIR}/skywire_${lang}")
    endif()
    file(MAKE_DIRECTORY "${out_dir}")

    set(namespace_args)
    if(arg_NAMESPACE)
        set(namespace_args --namespace "${arg_NAMESPACE}")
    endif()

    set(text_args)
    if(arg_JSON)
        list(APPEND text_args --generate_json)
    endif()
    if(arg_XML)
        list(APPEND text_args --generate_xml)
    endif()
    if(arg_BSON)
        list(APPEND text_args --generate_bson)
    endif()

    set(specs)
    foreach(spec IN LISTS arg_SPECS)
        get_filename_component(spec "${spec}" ABSOLUTE)
        list(APPEND specs "${spec}")
    endforeach()

    set(all_outputs)
    foreach(spec IN LISTS specs)
        # Output names depend on the spec's contents; reconfigure when it changes.
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${spec}")
        file(READ "${spec}" json)
        string(JSON classname ERROR_VARIABLE err GET "${json}" stringValues classname)
        if(err)
            message(FATAL_ERROR "skywire_generate: ${spec} has no stringValues.classname")
        endif()

        string(JSON header ERROR_VARIABLE err GET "${json}" stringValues outputheaderfilename)
        if(err)
            set(header "{classname}.h")
        endif()
        string(REPLACE "{classname}" "${classname}" header "${header}")
        set(outputs "${out_dir}/${header}")

        if(lang STREQUAL "c")
            string(JSON impl ERROR_VARIABLE err GET "${json}" stringValues outputimplfilename)
            if(err)
                set(impl "{classname}.c")
            endif()
            string(REPLACE "{classname}" "${classname}" impl "${impl}")
            list(APPEND outputs "${out_dir}/${impl}")
        endif()

        add_custom_command(
            OUTPUT  ${outputs}
            COMMAND skywire::wiregen
                    --language ${lang}
                    --input_file "${spec}"
                    --outdir "${out_dir}"
                    ${namespace_args}
                    ${text_args}
                    --write_to_file
            DEPENDS ${specs} skywire::wiregen
            COMMENT "wiregen ${lang}: ${classname}"
            VERBATIM)
        list(APPEND all_outputs ${outputs})
    endforeach()

    if(arg_TARGET)
        if(NOT TARGET skywire::${lang}_runtime)
            message(FATAL_ERROR "skywire_generate: ${arg_TARGET} needs skywire::${lang}_runtime, "
                                "which needs protozero; set PROTOZERO_DIR to the directory holding its include/")
        endif()
        target_sources(${arg_TARGET} PRIVATE ${all_outputs})
        target_include_directories(${arg_TARGET} PUBLIC "${out_dir}")
        target_link_libraries(${arg_TARGET} PUBLIC skywire::${lang}_runtime)
    endif()
    if(arg_OUT_VAR)
        set(${arg_OUT_VAR} ${all_outputs} PARENT_SCOPE)
    endif()
endfunction()
