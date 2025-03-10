# Copyright 2020 The Dawn & Tint Authors
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

###############################################################################
# Dawn headers
###############################################################################

DawnJSONGenerator(
    TARGET "headers"
    PRINT_NAME "Dawn headers"
    OUTPUT_HEADERS DAWN_HEADERS_GEN_HEADERS
)

dawn_add_library(
    dawn_headers
    HEADER_ONLY
    UTILITY_TARGET dawn_internal_config
    HEADERS
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu.h"
        ${DAWN_HEADERS_GEN_HEADERS}
    DEPENDS
        dawn_public_config
)

if (${DAWN_ENABLE_EMSCRIPTEN})
  add_library(webgpu_c ALIAS emdawnwebgpu_c)
else()
  add_library(webgpu_c ALIAS dawn_headers)
endif()

###############################################################################
# Dawn C++ headers
###############################################################################

DawnJSONGenerator(
    TARGET "cpp_headers"
    PRINT_NAME "Dawn C++ headers"
    OUTPUT_HEADERS DAWNCPP_HEADERS_GEN_HEADERS
)
dawn_add_library(
    dawncpp_headers
    HEADER_ONLY
    UTILITY_TARGET dawn_internal_config
    HEADERS
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu_cpp.h"
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu_enum_class_bitmasks.h"
        ${DAWNCPP_HEADERS_GEN_HEADERS}
    DEPENDS
        dawn::dawn_headers
)

if (${DAWN_ENABLE_EMSCRIPTEN})
  add_library(webgpu_cpp ALIAS emdawnwebgpu_cpp)
else()
  add_library(dawncpp ALIAS dawncpp_headers)
  add_library(webgpu_cpp ALIAS dawncpp_headers)
endif()

###############################################################################
# Dawn projects
###############################################################################

add_subdirectory(partition_alloc)
add_subdirectory(common)
add_subdirectory(utils)
# Only build the rest of Dawn when Emscripten is not the target environment.
if (NOT ${DAWN_ENABLE_EMSCRIPTEN})
  add_subdirectory(platform)
  add_subdirectory(wire)
  add_subdirectory(native)
endif()

if (DAWN_BUILD_BENCHMARKS)
  add_subdirectory(tests/benchmarks)
endif()
if(DAWN_USE_GLFW)
  add_subdirectory(glfw)
endif()
if (DAWN_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
if (DAWN_BUILD_NODE_BINDINGS)
  add_subdirectory(node)
endif()

###############################################################################
# libdawn_proc
#   Only built when not building for Emscripten
###############################################################################

if (NOT ${DAWN_ENABLE_EMSCRIPTEN})
  DawnJSONGenerator(
      TARGET "proc"
      PRINT_NAME "Dawn C++ wrapper"
      OUTPUT_SOURCES DAWNPROC_GEN_SOURCES
  )

  dawn_add_library(
    dawn_proc
    UTILITY_TARGET dawn_internal_config
    HEADERS
      "${DAWN_INCLUDE_DIR}/dawn/dawn_thread_dispatch_proc.h"
      "${DAWN_INCLUDE_DIR}/dawn/dawn_proc.h"
    SOURCES
      "${DAWNPROC_GEN_SOURCES}"
    DEPENDS
      dawn::dawn_headers
    PRIVATE_DEPENDS
      dawn::dawn_common
  )
  target_compile_definitions(dawn_proc PRIVATE "WGPU_IMPLEMENTATION")
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_proc PUBLIC "WGPU_SHARED_LIBRARY")
  endif()
endif()

###############################################################################
# Other generated files (upstream header, emscripten header, emscripten bits)
###############################################################################

DawnJSONGenerator(
    TARGET "webgpu_headers"
    PRINT_NAME "WebGPU headers"
    OUTPUT_HEADERS WEBGPU_HEADERS_GEN_HEADERS
)
add_custom_target(webgpu_headers_gen
    DEPENDS ${WEBGPU_HEADERS_GEN_HEADERS}
)
