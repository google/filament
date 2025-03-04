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
# Dawn WGPU utilities
#   - Used in tests and samples
###############################################################################
set(private_wgpu_depends)
if (TINT_BUILD_SPV_READER)
      list(APPEND private_wgpu_depends SPIRV-Tools-opt)
endif()

dawn_add_library(
  dawn_wgpu_utils
  ENABLE_EMSCRIPTEN
  UTILITY_TARGET dawn_internal_config
  PRIVATE_HEADERS
    "ComboRenderBundleEncoderDescriptor.h"
    "ComboRenderPipelineDescriptor.h"
    "TextureUtils.h"
    "WGPUHelpers.h"
  SOURCES
    "ComboRenderBundleEncoderDescriptor.cpp"
    "ComboRenderPipelineDescriptor.cpp"
    "TextureUtils.cpp"
    "WGPUHelpers.cpp"
  DEPENDS
    webgpu_cpp
    dawn::dawn_common
  PRIVATE_DEPENDS
    ${private_wgpu_depends}
)

# Needed by WGPUHelpers
target_compile_definitions(dawn_wgpu_utils
  PUBLIC
    "-DTINT_BUILD_SPV_READER=$<BOOL:${TINT_BUILD_SPV_READER}>")

###############################################################################
# Dawn test utilities
#   - Used in tests
###############################################################################
dawn_add_library(
  dawn_test_utils
  UTILITY_TARGET dawn_internal_config
  PRIVATE_HEADERS
    "BinarySemaphore.h"
    "TerribleCommandBuffer.h"
    "TestUtils.h"
    "WireHelper.h"
  SOURCES
    "BinarySemaphore.cpp"
    "TerribleCommandBuffer.cpp"
    "TestUtils.cpp"
    "WireHelper.cpp"
  DEPENDS
    webgpu_cpp
    dawn::dawn_common
    dawn::dawn_wire
    dawn::partition_alloc
)

###############################################################################
# Dawn system utilities
#   - Used in tests and samples
###############################################################################
set(private_system_headers
    "CommandLineParser.h"
    "PlatformDebugLogger.h"
    "SystemUtils.h"
    "Timer.h"
)
set(system_sources
    "CommandLineParser.cpp"
    "SystemUtils.cpp"
)
set(private_system_depends)

if (WIN32 AND NOT WINDOWS_STORE)
  list(APPEND system_sources "WindowsDebugLogger.cpp")
else ()
  list(APPEND system_sources "EmptyDebugLogger.cpp")
endif ()

if (WIN32)
  list(APPEND system_sources "WindowsTimer.cpp")
elseif (APPLE)
  list(APPEND private_system_headers
      "ObjCUtils.h"
  )
  list(APPEND system_sources
      "ObjCUtils.mm"
      "OSXTimer.cpp"
  )
  list(APPEND private_system_depends "-framework QuartzCore")
elseif (UNIX)
    list(APPEND system_sources "PosixTimer.cpp")
endif ()

dawn_add_library(
  dawn_system_utils
  ENABLE_EMSCRIPTEN
  UTILITY_TARGET dawn_internal_config
  PRIVATE_HEADERS
    ${private_system_headers}
  SOURCES
    ${system_sources}
  DEPENDS
    dawn::dawn_common
    absl::span
  PRIVATE_DEPENDS
    absl::strings
    absl::string_view
    absl::str_format_internal
    absl::flat_hash_map
    ${private_system_depends}
)
