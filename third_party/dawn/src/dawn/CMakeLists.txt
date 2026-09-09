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
# Dawn version
###############################################################################

DawnGenerator(
    SCRIPT "${Dawn_SOURCE_DIR}/generator/dawn_version_generator.py"
    PRINT_NAME "Dawn version based utilities"
    OUTPUT_HEADERS DAWN_VERSION_AUTOGEN_HEADERS
    EXTRA_PARAMETERS "--dawn-dir"
         "${Dawn_SOURCE_DIR}"
)

dawn_add_library(
    dawn_version
    HEADER_ONLY
    ENABLE_EMSCRIPTEN
    UTILITY_TARGET dawn_internal_config
    HEADERS
        ${DAWN_VERSION_AUTOGEN_HEADERS}
    DEPENDS
        dawn_public_config
)

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

if(BUILD_SHARED_LIBS)
  target_compile_definitions(dawn_headers INTERFACE "WGPU_SHARED_LIBRARY")
endif()

if (EMSCRIPTEN)
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
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu_cpp_print.h"
        "${DAWN_INCLUDE_DIR}/webgpu/webgpu_enum_class_bitmasks.h"
        ${DAWNCPP_HEADERS_GEN_HEADERS}
    DEPENDS
        dawn::dawn_headers
)

if (EMSCRIPTEN)
  add_library(webgpu_cpp ALIAS emdawnwebgpu_cpp)
  set(cpp_modules_gen_target emdawnwebgpu_modules)
  set(cpp_modules_includes "${DAWN_BUILD_GEN_DIR}/src/emdawnwebgpu/include")
else()
  add_library(dawncpp ALIAS dawncpp_headers)
  add_library(webgpu_cpp ALIAS dawncpp_headers)
  set(cpp_modules_gen_target cpp_modules)
  set(cpp_modules_includes "${DAWN_BUILD_GEN_DIR}/include")
endif()

# Dawn c++20 Module Interface
if(DAWN_SUPPORTS_CXX_MODULES)
  DawnJSONGenerator(
      TARGET ${cpp_modules_gen_target}
      PRINT_NAME "Dawn C++ module interface"
      OUTPUT_SOURCES DAWNCPP_MODULES_GEN_SOURCES
  )

  add_library(dawncpp_module OBJECT)
  add_library(dawn::cpp_module ALIAS dawncpp_module)
  target_sources(dawncpp_module PUBLIC
    FILE_SET  dawncpp_modules
    TYPE      CXX_MODULES
    BASE_DIRS "${DAWN_BUILD_GEN_DIR}"
    FILES     "${DAWNCPP_MODULES_GEN_SOURCES}"
  )
  target_include_directories(dawncpp_module PRIVATE
    "$<BUILD_INTERFACE:${cpp_modules_includes}>"
    "$<BUILD_INTERFACE:${DAWN_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
  add_dependencies(dawncpp_module webgpu_cpp webgpu_c)
  get_target_property(webgpu_cpp_ALIASED webgpu_cpp ALIASED_TARGET)
  target_link_libraries(${webgpu_cpp_ALIASED} INTERFACE dawncpp_module)

  # Use toplevel compile configuration for the interface
  # to avoid -Wmodule-file-config-mismatch
  # See https://gitlab.kitware.com/cmake/cmake/-/work_items/27597
  set_target_properties(dawncpp_module PROPERTIES
    CXX_STANDARD    ${DAWN_TOPLEVEL_CXX_STANDARD}
    CXX_EXTENSIONS  ${DAWN_TOPLEVEL_CXX_EXTENSIONS}
  )
  target_compile_features(dawncpp_module PUBLIC cxx_std_20)

  set_property(TARGET dawncpp_module PROPERTY EXPORT_NAME cpp_module)

  install(TARGETS dawncpp_module
    EXPORT DawnTargets
    FILE_SET dawncpp_modules
    DESTINATION "."
  )

  if (EMSCRIPTEN)
    # Add module interface to emdawnwebgpu_pkg
    add_custom_target(emdawnwebgpu_pkg_cppmodules
      COMMAND ${CMAKE_COMMAND} -E make_directory
        "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/webgpu/"

      COMMAND ${CMAKE_COMMAND} -E copy
        ${DAWNCPP_MODULES_GEN_SOURCES}
        "${CMAKE_BINARY_DIR}/emdawnwebgpu_pkg/webgpu_cpp/include/webgpu/"
    )
    add_dependencies(emdawnwebgpu_pkg emdawnwebgpu_pkg_cppmodules)
  endif()
endif()

###############################################################################
# Dawn projects
###############################################################################

add_subdirectory(partition_alloc)
add_subdirectory(common)
add_subdirectory(utils)
# Only build the rest of Dawn when Emscripten is not the target environment.
if (NOT EMSCRIPTEN)
  add_subdirectory(platform)
  add_subdirectory(wire)
  add_subdirectory(native)
  add_subdirectory(replay)
endif()

if (${DAWN_BUILD_TESTS})
  add_subdirectory(tests)
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

if (NOT EMSCRIPTEN)
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
