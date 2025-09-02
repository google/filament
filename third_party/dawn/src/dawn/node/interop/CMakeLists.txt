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

# Paths to generated files
set(INTEROP_GEN_DIR    "${DAWN_NODE_GEN_DIR}/src/dawn/node/interop")
set(INTEROP_WEBGPU_H   "${INTEROP_GEN_DIR}/WebGPU.h")
set(INTEROP_WEBGPU_CPP "${INTEROP_GEN_DIR}/WebGPU.cpp")

idlgen(
    TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPU.h.tmpl"
    IDLS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser.idl"
        "${WEBGPU_IDL_PATH}"
        "${CMAKE_CURRENT_SOURCE_DIR}/DawnExtensions.idl"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPUCommon.tmpl"
    OUTPUT
        "${INTEROP_WEBGPU_H}"
)

idlgen(
    TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPU.cpp.tmpl"
    IDLS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser.idl"
        "${WEBGPU_IDL_PATH}"
        "${CMAKE_CURRENT_SOURCE_DIR}/DawnExtensions.idl"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/WebGPUCommon.tmpl"
    OUTPUT
        "${INTEROP_WEBGPU_CPP}"
)

add_library(dawn_node_interop STATIC
    "Core.cpp"
    "Core.h"
    "${INTEROP_WEBGPU_H}"
    "${INTEROP_WEBGPU_CPP}"
)
common_compile_options(dawn_node_interop)

target_include_directories(dawn_node_interop
    PRIVATE
        "${PROJECT_SOURCE_DIR}"
        "${DAWN_NODE_GEN_DIR}"
)
target_include_directories(dawn_node_interop SYSTEM
    PRIVATE
        "${NODE_ADDON_API_DIR}"
        "${NODE_API_HEADERS_DIR}/include"
)

target_link_libraries(dawn_node_interop
    PRIVATE
        dawncpp
        dawn_warnings_config
)
