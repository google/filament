# Copyright 2021 The Dawn & Tint Authors
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

set(NODE_BINDING_DEPS
    ${NODE_ADDON_API_DIR}
    ${NODE_API_HEADERS_DIR}
    ${WEBGPU_IDL_PATH}
)
foreach(DEP ${NODE_BINDING_DEPS})
    if (NOT EXISTS ${DEP})
        message(FATAL_ERROR
            "DAWN_BUILD_NODE_BINDINGS requires missing dependency '${DEP}'\n"
            "Please follow the 'Fetch dependencies' instructions at:\n"
            "./src/dawn/node/README.md"
        )
    endif()
endforeach()
if (NOT CMAKE_POSITION_INDEPENDENT_CODE)
    message(FATAL_ERROR "DAWN_BUILD_NODE_BINDINGS requires building with DAWN_ENABLE_PIC")
endif()

set(DAWN_NODE_GEN_DIR "${DAWN_BUILD_GEN_DIR}/node")
set(IDLGEN_TOOL_DIR   "${PROJECT_SOURCE_DIR}/tools/src/cmd/idlgen")

# idlgen() is a function that uses the tools/src/cmd/idlgen/main.go tool to
# generate code from an IDL file and template.
# idlgen() accepts the following named arguments:
#   TEMPLATE <path> - (required) the path to the root .tmpl file. If the
#                     template imports other templates, then these should be
#                     added to the DEPENDS argument list.
#   OUTPUT <path>   - (required) the output file path.
#   IDLS <paths>    - (at least one required) the list of input WebIDL files.
#   DEPENDS <paths> - an optional list of additional file dependencies used.
function(idlgen)
    cmake_parse_arguments(IDLGEN
        ""                # options
        "TEMPLATE;OUTPUT" # one_value_keywords
        "IDLS;DEPENDS"    # multi_value_keywords
        ${ARGN})

    if(NOT IDLGEN_TEMPLATE)
        message(FATAL_ERROR "idlgen() missing TEMPLATE argument")
    endif()
    if(NOT IDLGEN_OUTPUT)
        message(FATAL_ERROR "idlgen() missing OUTPUT argument")
    endif()
    if(NOT IDLGEN_IDLS)
        message(FATAL_ERROR "idlgen() missing IDLS argument(s)")
    endif()
    add_custom_command(
        COMMAND ${GO_EXECUTABLE} "run" "${IDLGEN_TOOL_DIR}/main.go"
                "--template" "${IDLGEN_TEMPLATE}"
                "--output"   "${IDLGEN_OUTPUT}"
                ${IDLGEN_IDLS}
        DEPENDS "${IDLGEN_TOOL_DIR}/main.go"
                ${IDLGEN_TEMPLATE}
                ${IDLGEN_DEPENDS}
                ${IDLGEN_IDLS}
        OUTPUT  ${IDLGEN_OUTPUT}
        WORKING_DIRECTORY ${IDLGEN_TOOL_DIR}
        COMMENT "Generating ${IDLGEN_OUTPUT}"
    )
endfunction()

add_subdirectory(binding)
add_subdirectory(interop)

add_library(dawn_node SHARED
    "Module.cpp"
)
common_compile_options(dawn_node)
set_target_properties(dawn_node PROPERTIES
    PREFIX ""
    OUTPUT_NAME "dawn"
    SUFFIX ".node"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
    CXX_STANDARD 17
)
target_link_libraries(dawn_node dawn_node_binding dawn_node_interop dawn_native dawncpp dawn_proc
                      libtint)
target_include_directories(dawn_node PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${NODE_ADDON_API_DIR}"
    "${NODE_API_HEADERS_DIR}/include"
    "${DAWN_NODE_GEN_DIR}"
)

# To reduce the build dependencies for compiling the dawn.node targets, we do
# not use cmake-js for building, but instead just depend on node_api_headers.
# As the name suggests, node_api_headers contains just the *headers* of Napi,
# and does not provide a library to link against.
# Fortunately node_api_headers provides a list of Napi symbols exported by Node,
# which we can use to either produce weak-symbol stubs (unix) or generate a .lib
# (Windows).

# Parse the Napi symbols from ${NODE_API_HEADERS_DIR}/symbols.js
file(READ "${NODE_API_HEADERS_DIR}/symbols.js" NAPI_SYMBOLS_JS_CONTENT)
string(REGEX MATCHALL "napi_[a-z0-9_]*" NAPI_SYMBOLS "${NAPI_SYMBOLS_JS_CONTENT}")

if (WIN32)
    set(NODE_API_BINARY_FILE "node.exe" CACHE STRING
       "The name of the file in which to find the Node-API symbols."
    )

    # Generate the NapiSymbols.def file from the Napi symbol list
    set(NAPI_SYMBOLS_DEF "${DAWN_NODE_GEN_DIR}/NapiSymbols.def")
    list(TRANSFORM NAPI_SYMBOLS PREPEND "  ")
    list(TRANSFORM NAPI_SYMBOLS APPEND "\n")
    string(REPLACE ";" "" NAPI_SYMBOLS "${NAPI_SYMBOLS}")
    string(PREPEND NAPI_SYMBOLS "LIBRARY ${NODE_API_BINARY_FILE}\nEXPORTS\n")
    file(GENERATE OUTPUT "${NAPI_SYMBOLS_DEF}" CONTENT "${NAPI_SYMBOLS}")
    # Generate the NapiSymbols.lib from the NapiSymbols.def file
    set(NAPI_SYMBOLS_LIB "${DAWN_NODE_GEN_DIR}/NapiSymbols.lib")
    # Resolve path to lib.exe
    get_filename_component(LINKER_BIN_DIR "${CMAKE_LINKER}" DIRECTORY)
    if (EXISTS "${LINKER_BIN_DIR}/lib.exe")
        set(LIB_EXE "${LINKER_BIN_DIR}/lib.exe")
    elseif (EXISTS "${LINKER_BIN_DIR}/lld-link.exe")
        set(LIB_EXE "${LINKER_BIN_DIR}/lld-link.exe")
    else()
        message(FATAL_ERROR "unable to find lib.exe or lld-link.exe")
    endif()
    add_custom_command(
        COMMAND "${LIB_EXE}"
                "/DEF:${NAPI_SYMBOLS_DEF}"
                "/OUT:${NAPI_SYMBOLS_LIB}"
        DEPENDS "${NAPI_SYMBOLS_DEF}"
        OUTPUT  "${NAPI_SYMBOLS_LIB}"
        COMMENT "Generating ${NAPI_SYMBOLS_LIB}"
    )
    add_custom_target(napi-symbols DEPENDS "${NAPI_SYMBOLS_LIB}")
    add_dependencies(dawn_node napi-symbols)
    target_link_libraries(dawn_node "${NAPI_SYMBOLS_LIB}")
else()
    # Generate the NapiSymbols.h file from the Napi symbol list
    set(NAPI_SYMBOLS_H "${DAWN_NODE_GEN_DIR}/NapiSymbols.h")
    list(TRANSFORM NAPI_SYMBOLS PREPEND "NAPI_SYMBOL(")
    list(TRANSFORM NAPI_SYMBOLS APPEND ")\n")
    string(REPLACE ";" "" NAPI_SYMBOLS "${NAPI_SYMBOLS}")
    file(GENERATE OUTPUT "${NAPI_SYMBOLS_H}" CONTENT "${NAPI_SYMBOLS}")
    target_sources(dawn_node PRIVATE "NapiSymbols.cpp")
endif()

macro(javascript file)
    add_custom_command(
        TARGET dawn_node POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/src/dawn/node/${file}"
            "$<TARGET_FILE_DIR:dawn_node>/${file}")
endmacro()

javascript("index.js")
javascript("cts.js")
