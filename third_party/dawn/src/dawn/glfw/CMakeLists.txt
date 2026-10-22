# Copyright 2022 The Dawn & Tint Authors
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

set(headers
    "${DAWN_INCLUDE_DIR}/webgpu/webgpu_glfw.h"
)
set(sources)
set(conditional_public_depends)
if (EMSCRIPTEN)
    list(APPEND sources "utils_emscripten.cpp")
    list(APPEND conditional_public_depends "emdawnwebgpu_cpp")
else()
    list(APPEND sources "utils.cpp")
    list(APPEND conditional_public_depends "dawn::dawn_common")
    list(APPEND conditional_public_depends "dawn::dawncpp_headers")
    if (DAWN_ENABLE_METAL)
        list(APPEND sources "utils_metal.mm")
        list(APPEND conditional_public_depends "-framework Metal")
        list(APPEND conditional_public_depends "-framework QuartzCore")
    endif ()
endif ()

dawn_add_library(
  dawn_glfw
  ENABLE_EMSCRIPTEN
  UTILITY_TARGET dawn_internal_config
  HEADERS
    "${headers}"
  SOURCES
    "${sources}"
  DEPENDS
    ${conditional_public_depends}
  PRIVATE_DEPENDS
    glfw
)
target_compile_definitions(dawn_glfw PRIVATE "WGPU_GLFW_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_glfw PUBLIC "WGPU_GLFW_SHARED_LIBRARY")
endif()

add_library(webgpu_glfw ALIAS dawn_glfw)
