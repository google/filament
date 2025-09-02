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

set(EM_BUILD_GEN_DIR "${DAWN_BUILD_GEN_DIR}/src/emdawnwebgpu")

DawnJSONGenerator(
    TARGET "emdawnwebgpu_headers"
    PRINT_NAME "Dawn WebGPU Emscripten headers"
    OUTPUT_HEADERS EMDAWNWEBGPU_HEADERS_GEN_HEADERS
)
add_custom_target(emdawnwebgpu_headers_gen
    DEPENDS ${EMDAWNWEBGPU_HEADERS_GEN_HEADERS}
)
# Also copy the webgpu_glfw.h file.
add_custom_command(TARGET emdawnwebgpu_headers_gen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu_glfw.h"
        "${EM_BUILD_GEN_DIR}/include/webgpu/"
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
if (EMSCRIPTEN)
    set(EM_SRC_DIR "${DAWN_SRC_DIR}/emdawnwebgpu")

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

        # Note this requires Emscripten 4.0.3+ (prior to that it was located
        # at tools/maint/gen_struct_info.py).
        set(ARGS
            ${Python3_EXECUTABLE}
            "${EMSCRIPTEN_ROOT_PATH}/tools/gen_struct_info.py"
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
            DEPENDS
                "${EMSCRIPTEN_ROOT_PATH}/tools/gen_struct_info.py"
                ${EMDAWNWEBGPU_HEADERS_GEN_HEADERS}  # for webgpu.h
                ${EMDAWNWEBGPU_STRUCT_INFO_JSON_GEN_SOURCES}  # for struct_info_webgpu.json
        )
    endfunction()

    message(STATUS "Dawn: Configuring Emscripten gen_struct_info for WebGPU.")

    webgpu_gen_struct_info(
        OUTPUT_JSON WEBGPU_STRUCT_INFO_GEN32
        WASM64 OFF
    )
    webgpu_gen_struct_info(
        OUTPUT_JSON WEBGPU_STRUCT_INFO_GEN64
        WASM64 ON
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
        set(SRCS
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part1.txt"
            "${WEBGPU_STRUCT_INFO_GEN32}"
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part2.txt"
            "${WEBGPU_STRUCT_INFO_GEN64}"
            "${EM_SRC_DIR}/snippets/library_webgpu_struct_info_part3.txt"
        )
        set(ARGS
            ${Python3_EXECUTABLE}
            "${EM_SRC_DIR}/concat.py"
            "${OUTPUT}"
            ${SRCS}
        )
        set(${arg_OUTPUT_JS} "${OUTPUT}" PARENT_SCOPE)

        add_custom_command(
            COMMAND ${ARGS}
            OUTPUT ${OUTPUT}
            COMMENT "Dawn Emscripten: Generating ${PRINT_NAME}."
            DEPENDS
                "${EM_SRC_DIR}/concat.py"
                ${SRCS}
        )
    endfunction()

    webgpu_gen_struct_info_js(
        OUTPUT_JS EMDAWNWEBGPU_STRUCT_INFO_JS
    )

    # Include dir, and dependency on the generated files in the include dir.
    add_library(emdawnwebgpu_c_include INTERFACE)
    target_include_directories(emdawnwebgpu_c_include BEFORE INTERFACE
        "${EM_BUILD_GEN_DIR}/include"
    )
    add_dependencies(emdawnwebgpu_c_include emdawnwebgpu_headers_gen)

    dawn_add_library(
        emdawnwebgpu_c
        ENABLE_EMSCRIPTEN
        HEADERS
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu.h"
        SOURCES
            "${DAWN_EMDAWNWEBGPU_DIR}/pkg/webgpu/src/webgpu.cpp"
        DEPENDS
            emdawnwebgpu_c_include
    )
    target_link_options(emdawnwebgpu_c INTERFACE
        # IMPORTANT: If changing these dependencies, update EMDAWNWEBGPU_C_ALL_FILE_DEPS too.
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_enum_tables.js"
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_generated_sig_info.js"
        "--js-library=${EM_BUILD_GEN_DIR}/library_webgpu_generated_struct_info.js"
        "--js-library=${DAWN_EMDAWNWEBGPU_DIR}/pkg/webgpu/src/library_webgpu.js"
        "--closure-args=--externs=${DAWN_EMDAWNWEBGPU_DIR}/pkg/webgpu/src/webgpu-externs.js"
    )

    # Dependencies from emdawnwebgpu_c to the files referenced in the linker flags.
    # - These are used as both file-level dependencies (defining *when* to rebuild) and
    #   target-level dependencies (defining *how* to rebuild).
    set(EMDAWNWEBGPU_C_GENERATED_FILE_DEPS
        "${EMDAWNWEBGPU_JS_GEN_SOURCES}"
        "${EMDAWNWEBGPU_STRUCT_INFO_JS}"
    )
    # - There are also source files, which are used only as file-level dependencies.
    set(EMDAWNWEBGPU_C_ALL_FILE_DEPS
        "${EMDAWNWEBGPU_C_GENERATED_FILE_DEPS}"
        "${DAWN_EMDAWNWEBGPU_DIR}/pkg/webgpu/src/library_webgpu.js"
        "${DAWN_EMDAWNWEBGPU_DIR}/pkg/webgpu/src/webgpu-externs.js"
    )
    add_custom_target(emdawnwebgpu_config_generated_deps
        DEPENDS "${EMDAWNWEBGPU_C_GENERATED_FILE_DEPS}"
    )
    add_dependencies(emdawnwebgpu_c emdawnwebgpu_config_generated_deps)
    set_target_properties(emdawnwebgpu_c PROPERTIES
        # This defines file-level dependencies of the linker (intended for "linker scripts").
        INTERFACE_LINK_DEPENDS "${EMDAWNWEBGPU_C_ALL_FILE_DEPS}"
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

    add_custom_target(emdawnwebgpu_pkg
        DEPENDS
            # This will compile webgpu.cpp, which is unnecessary because we package webgpu.cpp as a
            # source file, but it's simpler than specifying all the generator dependencies again.
            emdawnwebgpu_c
    )

    add_custom_command(TARGET emdawnwebgpu_pkg POST_BUILD
        # Copy files from src/emdawnwebgpu/pkg and third_party/emdawnwebgpu/pkg
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${DAWN_EMDAWNWEBGPU_DIR}/pkg/"
            "${EM_SRC_DIR}/pkg/"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/"
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu/include/webgpu/"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/webgpu/"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/dawn/"

        # Copy webgpu_enum_class_bitmasks.h
        COMMAND ${CMAKE_COMMAND} -E copy
            "${DAWN_INCLUDE_DIR}/webgpu/webgpu_enum_class_bitmasks.h"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/webgpu/"

        # Copy webgpu_glfw.h
        COMMAND ${CMAKE_COMMAND} -E copy
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu_glfw.h"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu/include/webgpu/"

        # Copy generated files
        COMMAND ${CMAKE_COMMAND} -E copy
            "${EM_BUILD_GEN_DIR}/library_webgpu_enum_tables.js"
            "${EM_BUILD_GEN_DIR}/library_webgpu_generated_struct_info.js"
            "${EM_BUILD_GEN_DIR}/library_webgpu_generated_sig_info.js"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu/src/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu.h"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu/include/webgpu/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu_cpp.h"
            "${EM_BUILD_GEN_DIR}/include/webgpu/webgpu_cpp_chained_struct.h"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/webgpu/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${EM_BUILD_GEN_DIR}/include/dawn/webgpu_cpp_print.h"
            "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/dawn/"
    )

    if (${DAWN_BUILD_TESTS})
        set(emdawnwebgpu_test_sources
            "tests/FuturesTests.cpp"
            "tests/SpotTests.cpp"
        )
        set(emdawnwebgpu_test_deps
            dawn::dawn_wgpu_utils
            emdawnwebgpu_cpp
            gmock_main_wasmsafe
        )

        add_library(emdawnwebgpu_test_linkopts INTERFACE)
        target_link_options(emdawnwebgpu_test_linkopts
            INTERFACE
                "-sALLOW_MEMORY_GROWTH"
                # Build with Closure so we can look at code size, and to test that Closure
                # minification works, and to let Closure statically analyze our JS.
                "$<$<CONFIG:Release,MinSizeRel>:--closure=1>"
        )
        if (NOT DAWN_ENABLE_ASAN)
            target_link_options(emdawnwebgpu_test_linkopts
                INTERFACE
                    # If ASAN is not enabled, enable SAFE_HEAP to catch some bugs.
                    # (ASAN+UBSAN is preferable to this.)
                    "$<$<CONFIG:Debug>:-sSAFE_HEAP>"
            )
        endif()

        add_library(emdawnwebgpu_tests)
        target_link_libraries(
            emdawnwebgpu_tests
            PUBLIC
                emdawnwebgpu_test_linkopts
                ${emdawnwebgpu_test_deps}
        )

        add_executable(emdawnwebgpu_tests_asyncify ${emdawnwebgpu_test_sources})
        set_target_properties(emdawnwebgpu_tests_asyncify PROPERTIES
            SUFFIX ".html")
        target_link_libraries(emdawnwebgpu_tests_asyncify PUBLIC emdawnwebgpu_tests)
        target_link_options(emdawnwebgpu_tests_asyncify
            PUBLIC
                # We need Asyncify or JSPI for Future tests.
                "-sASYNCIFY=1"
                "-sASYNCIFY_STACK_SIZE=10000"  # Needed when building with ASAN.
        )

        add_executable(emdawnwebgpu_tests_jspi ${emdawnwebgpu_test_sources})
        set_target_properties(emdawnwebgpu_tests_jspi PROPERTIES
            SUFFIX ".html")
        target_link_libraries(emdawnwebgpu_tests_jspi PUBLIC emdawnwebgpu_tests)
        target_link_options(emdawnwebgpu_tests_jspi
            PUBLIC
                # We need Asyncify or JSPI for Future tests.
                "-sJSPI=1"
        )

        # A "sample" that makes real (bogus) API calls to serve as a basic code size test.
        DawnJSONGenerator(
            TARGET "emdawnwebgpu_link_test_cpp"
            PRINT_NAME "emdawnwebgpu LinkTest.cpp"
            OUTPUT_SOURCES EMDAWNWEBGPU_LINK_TEST_CPP_SOURCES
        )
        add_executable(emdawnwebgpu_link_test
            ${EMDAWNWEBGPU_LINK_TEST_CPP_SOURCES}
        )
        # The test is just that this links, not that it runs (it will just
        # crash), so just build to .js. Since it has a main() function, this is
        # the same as building to .html, but skipping the .html file.
        set_target_properties(emdawnwebgpu_link_test PROPERTIES
            SUFFIX ".js")
        target_link_libraries(
            emdawnwebgpu_link_test
            PUBLIC
                emdawnwebgpu_test_linkopts
                emdawnwebgpu_c
        )

        # A "sample" that just creates a device. This is used to check the code size for the
        # elimination of unused library code.
        add_executable(emdawnwebgpu_init_only_sample
            "InitOnlySample.cpp"
        )
        set_target_properties(emdawnwebgpu_init_only_sample PROPERTIES
            SUFFIX ".js")
        target_link_libraries(
            emdawnwebgpu_init_only_sample
            PUBLIC
                emdawnwebgpu_test_linkopts
                emdawnwebgpu_cpp
        )
    endif()
endif()
