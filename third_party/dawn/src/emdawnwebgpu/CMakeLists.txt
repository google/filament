# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

DawnJSONGenerator(
    TARGET "emdawnwebgpu_headers"
    PRINT_NAME "Dawn WebGPU Emscripten headers"
    OUTPUT_HEADERS EMDAWNWEBGPU_HEADERS_GEN_HEADERS
)
add_custom_target(emdawnwebgpu_headers_gen
    DEPENDS ${EMDAWNWEBGPU_HEADERS_GEN_HEADERS}
)

DawnJSONGenerator(
    TARGET "emdawnwebgpu_js"
    PRINT_NAME "Dawn WebGPU Emscripten JS files"
    OUTPUT_JSONS EMDAWNWEBGPU_STRUCT_INFO_JSON_GEN_SOURCES
    OUTPUT_JS EMDAWNWEBGPU_JS_GEN_SOURCES
)
add_custom_target(emdawnwebgpu_js_gen
    DEPENDS ${EMDAWNWEBGPU_STRUCT_INFO_JSON_GEN_SOURCES} ${EMDAWNWEBGPU_JS_GEN_SOURCES}
)

# When Emscripten is available, we can use one of its helper scripts to generate
# the struct info needed for our bindings fork (third_party/emdawnwebgpu).
# Those helpers, and their tree of generated dependencies, are:
#
# - library_webgpu_generated_struct_info.js
#   is constructed by concatenating:
#     - Some glue "snippets" from txt files
#     - webgpu_generated_struct_info{32,64}.json
#       which are generated using an Emscripten tool "gen_struct_info.py", from:
#         - webgpu.h (generated from dawn.json)
#         - struct_info_webgpu.json (generated from dawn.json)
#
# The bindings also require the following helpers (generated above):
#
# - library_webgpu_enum_tables.js
# - library_webgpu_generated_sig_info.js
#   which we generate directly instead of using "gen_sig_info.py"
if (${DAWN_ENABLE_EMSCRIPTEN})
    set(EM_SRC_DIR "${DAWN_SRC_DIR}/emdawnwebgpu")
    set(EM_BUILD_GEN_DIR "${DAWN_BUILD_GEN_DIR}/src/emdawnwebgpu")

    function(webgpu_gen_struct_info)
        cmake_parse_arguments(PARSE_ARGV 0 arg
            ""
            "OUTPUT_JSON;WASM64"
            ""
        )
        if (arg_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR
                "Unparsed arguments for webgpu_gen_struct_info: "
                "${arg_UNPARSED_ARGUMENTS}")
        endif()

        set(ARGS
            ${Python3_EXECUTABLE}
            "${DAWN_EMSCRIPTEN_TOOLCHAIN}/tools/maint/gen_struct_info.py"
            -q
            "${EM_BUILD_GEN_DIR}/struct_info_webgpu.json"
            "-I=${EM_BUILD_GEN_DIR}/include"
        )

        set(PRINT_NAME)
        if (${arg_WASM64})
            set(PRINT_NAME "webgpu_generated_struct_info64.json")
            list(APPEND ARGS "--wasm64")
        else()
            set(PRINT_NAME "webgpu_generated_struct_info32.json")
        endif()
        set(OUTPUT "${EM_BUILD_GEN_DIR}/${PRINT_NAME}")
        set(${arg_OUTPUT_JSON} "${OUTPUT}" PARENT_SCOPE)
        list(APPEND ARGS "-o=${OUTPUT}")

        add_custom_command(
            COMMAND ${ARGS}
            OUTPUT ${OUTPUT}
            COMMENT "Dawn Emscripten: Generating ${PRINT_NAME}."
        )

        # Prior to CMake 3.20 the GENERATED property is local to a directory which means that putting
        # generated headers in INTERFACE properties causes dependent targets to complain that they
        # cannot find the file. (because they don't see it as generated and want to check is is
        # actually on the filesystem).
        # Work around this by generating the files once if they aren't present at configuration time.
        if (NOT EXISTS ${OUTPUT})
            message(STATUS "Dawn: Generating initial versions of files for ${PRINT_NAME}.")
            execute_process(COMMAND ${ARGS} RESULT_VARIABLE RET)
            if (NOT RET EQUAL 0)
                message(FATAL_ERROR "Dawn: Failed to generate the initial version of files for ${PRINT_NAME}. Base args are '${ARGS}'.")
            endif()
        endif()
    endfunction()

    message(STATUS "Dawn: Configuring Emscripten gen_struct_info for WebGPU.")

    webgpu_gen_struct_info(
        OUTPUT_JSON WEBGPU_STRUCT_INFO_GEN32
        WASM64 OFF
    )
    add_custom_target(webgpu_generated_struct_info32
        DEPENDS emdawnwebgpu_headers_gen ${EMDAWNWEBGPU_STRUCT_INFO_JSON_GEN_SOURCES} ${WEBGPU_STRUCT_INFO_GEN32}
    )
    webgpu_gen_struct_info(
        OUTPUT_JSON WEBGPU_STRUCT_INFO_GEN64
        WASM64 ON
    )
    add_custom_target(webgpu_generated_struct_info64
        DEPENDS emdawnwebgpu_headers_gen ${EMDAWNWEBGPU_STRUCT_INFO_JSON_GEN_SOURCES} ${WEBGPU_STRUCT_INFO_GEN64}
    )

    # TODO(crbug.com/346806934): Consider using CMake builtin `cat` as per https://stackoverflow.com/a/62362885,
    # especially if we are to remove the GN build where concat.py is needed.
    function(webgpu_gen_struct_info_js)
        cmake_parse_arguments(PARSE_ARGV 0 arg
        ""
        "OUTPUT_JS"
        ""
        )
        if (arg_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR
                "Unparsed arguments for webgpu_gen_struct_info_js: "
                "${arg_UNPARSED_ARGUMENTS}")
        endif()

        set(PRINT_NAME library_webgpu_generated_struct_info.js)
        set(OUTPUT "${EM_BUILD_GEN_DIR}/${PRINT_NAME}")
        set(ARGS
            ${Python3_EXECUTABLE}
            "${EM_SRC_DIR}/concat.py"
            "${OUTPUT}"
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part1.txt"
            "${EM_BUILD_GEN_DIR}/webgpu_generated_struct_info32.json"
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part2.txt"
            "${EM_BUILD_GEN_DIR}/webgpu_generated_struct_info64.json"
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part3.txt"
        )
        set(${arg_OUTPUT_JS} "${OUTPUT}" PARENT_SCOPE)

        add_custom_command(
            COMMAND ${ARGS}
            OUTPUT ${OUTPUT}
            COMMENT "Dawn Emscripten: Generating ${PRINT_NAME}."
        )

        # Prior to CMake 3.20 the GENERATED property is local to a directory which means that putting
        # generated headers in INTERFACE properties causes dependent targets to complain that they
        # cannot find the file. (because they don't see it as generated and want to check is is
        # actually on the filesystem).
        # Work around this by generating the files once if they aren't present at configuration time.
        if (NOT EXISTS ${OUTPUT})
            message(STATUS "Dawn: Generating initial versions of files for ${PRINT_NAME}.")
            execute_process(COMMAND ${ARGS} RESULT_VARIABLE RET)
            if (NOT RET EQUAL 0)
                message(FATAL_ERROR "Dawn: Failed to generate the initial version of files for ${PRINT_NAME}. Base args are '${ARGS}'.")
            endif()
        endif()
    endfunction()

    webgpu_gen_struct_info_js(
        OUTPUT_JS EMDAWNWEBGPU_STRUCT_INFO_JS
    )
    add_custom_target(webgpu_generated_struct_info_js
        DEPENDS webgpu_generated_struct_info32 webgpu_generated_struct_info64 ${EMDAWNWEBGPU_STRUCT_INFO_JS}
    )

    add_library(emdawnwebgpu_config INTERFACE)
    target_include_directories(emdawnwebgpu_config BEFORE INTERFACE
        "${EM_BUILD_GEN_DIR}/include"
    )
    add_dependencies(emdawnwebgpu_config webgpu_generated_struct_info_js emdawnwebgpu_js_gen)
    target_link_options(emdawnwebgpu_config INTERFACE
        # We are using Dawn-generated bindings, not built-in ones
        "-sUSE_WEBGPU=0"
        # The JS libraries needed for bindings
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_enum_tables.js"
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_generated_struct_info.js"
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_generated_sig_info.js"
        "--js-library=${DAWN_EMDAWNWEBGPU_DIR}/library_webgpu.js"
        "--closure-args=--externs=${DAWN_EMSCRIPTEN_TOOLCHAIN}/src/closure-externs/webgpu-externs.js"
    )

    dawn_add_library(
        emdawnwebgpu_c
        ENABLE_EMSCRIPTEN
        HEADERS
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu.h"
        SOURCES
            "${DAWN_EMDAWNWEBGPU_DIR}/webgpu.cpp"
        DEPENDS
            emdawnwebgpu_config
    )

    dawn_add_library(
        emdawnwebgpu_cpp
        ENABLE_EMSCRIPTEN
        HEADER_ONLY
        HEADERS
            "${EM_BUILD_GEN_DIR}/include/dawn/webgpu_cpp_print.h"
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu_cpp.h"
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu_cpp_chained_struct.h"
            "${DAWN_INCLUDE_DIR}/webgpu/webgpu_enum_class_bitmasks.h"
        DEPENDS
            emdawnwebgpu_c
    )

    if (${DAWN_BUILD_TESTS})
        set(emdawnwebgpu_test_sources
            "tests/FuturesTests.cpp"
        )
        set(emdawnwebgpu_test_deps
            dawn::dawn_wgpu_utils
            emdawnwebgpu_cpp
            gmock_main
        )

        add_executable(emdawnwebgpu_tests_asyncify ${emdawnwebgpu_test_sources})
        set_target_properties(emdawnwebgpu_tests_asyncify PROPERTIES
            SUFFIX ".html")
        target_link_libraries(
            emdawnwebgpu_tests_asyncify
            PUBLIC
                ${emdawnwebgpu_test_deps}
        )
        target_link_options(emdawnwebgpu_tests_asyncify PUBLIC
            # We need Asyncify or JSPI for Future tests.
            "-sASYNCIFY=1"
        )

        add_executable(emdawnwebgpu_tests_jspi ${emdawnwebgpu_test_sources})
        set_target_properties(emdawnwebgpu_tests_jspi PROPERTIES
            SUFFIX ".html")
        target_link_libraries(
            emdawnwebgpu_tests_jspi
            PUBLIC
                ${emdawnwebgpu_test_deps}
        )
        target_link_options(emdawnwebgpu_tests_jspi PUBLIC
            # We need Asyncify or JSPI for Future tests.
            "-sJSPI=1"
        )
    endif()
endif()
