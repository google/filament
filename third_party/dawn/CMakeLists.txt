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

# As per https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md
cmake_minimum_required(VERSION 3.22)

# - Since we are past CMake 3.12 we can add CONFIGURE_DEPENDS to DawnGenerator to rerun CMake
#   in case any of the generator files changes. We should also remove the CACHE "" FORCE stuff to
#   override options in third_party dependencies.

project(
    Dawn
    DESCRIPTION "Dawn, a WebGPU implementation"
    LANGUAGES C CXX
    HOMEPAGE_URL "https://dawn.googlesource.com/dawn"
    VERSION 0.0.0
)
enable_testing()

list(INSERT CMAKE_MODULE_PATH 0 "${Dawn_SOURCE_DIR}/src/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IS_DEBUG_BUILD 0)
string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
if ((NOT ${build_type} STREQUAL "RELEASE") AND (NOT ${build_type} STREQUAL "RELWITHDEBINFO"))
  set(IS_DEBUG_BUILD 1)
endif()

include(DawnSetIfNotDefined)

set(DAWN_BUILD_GEN_DIR "${Dawn_BINARY_DIR}/gen" CACHE PATH "Directory that contains generated source files")
set(DAWN_GENERATOR_DIR "${Dawn_SOURCE_DIR}/generator" CACHE PATH "Directory that contains scripts to generate sources and headers")
set(DAWN_SRC_DIR "${Dawn_SOURCE_DIR}/src" CACHE PATH "Directory that contains source files for dawn")
set(DAWN_INCLUDE_DIR "${Dawn_SOURCE_DIR}/include" CACHE PATH "Directory that contains public headers for dawn")
set(DAWN_TEMPLATE_DIR "${DAWN_GENERATOR_DIR}/templates" CACHE PATH "Directory that contains templates for generators")

# Emscripten.cmake toolchain sets this variable to `0`; otherwise, it's undefined, so set it to 1.
set_if_not_defined(EMSCRIPTEN 0 "1 if building with Emscripten")

################################################################################
# Configuration options
################################################################################
# Default values for the backend-enabling options
set(ENABLE_D3D11 OFF)
set(ENABLE_D3D12 OFF)
set(ENABLE_METAL OFF)
set(ENABLE_NULL ON)
set(ENABLE_WEBGPU_ON_WEBGPU OFF)
set(ENABLE_OPENGLES OFF)
set(ENABLE_DESKTOP_GL OFF)
set(ENABLE_VULKAN OFF)
set(ENABLE_SPIRV_VALIDATION OFF)
set(USE_WAYLAND OFF)
set(USE_X11 OFF)
set(USE_WINDOWS_UI OFF)
set(BUILD_SAMPLES OFF)
set(BUILD_TESTS OFF)
set(TARGET_MACOS OFF)
set(FORCE_SYSTEM_COMPONENT_LOAD OFF)
if (EMSCRIPTEN)
  # Only the samples are supported for Emscripten at the moment.
  # TODO(crbug.com/42240181): Make dawn_end2end_tests work too.
  set(BUILD_SAMPLES ON)
  set(BUILD_TESTS ON)
  set(ENABLE_NULL OFF)
  set(ENABLE_WEBGPU_ON_WEBGPU OFF)
elseif (WIN32)
  set(ENABLE_D3D11 ON)
  set(USE_WINDOWS_UI ON)
  set(ENABLE_D3D12 ON)
  if (NOT WINDOWS_STORE)
    # Enable Vulkan in win32 compilation only
    # since UWP only supports d3d
    set(ENABLE_VULKAN ON)
    set(ENABLE_SPIRV_VALIDATION ON)
  endif()
elseif(APPLE)
  set(ENABLE_METAL ON)
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin" OR CMAKE_SYSTEM_NAME MATCHES "MacOS")
    set(TARGET_MACOS ON)
  endif()
elseif(ANDROID)
  set(ENABLE_VULKAN ON)
  set(ENABLE_OPENGLES ON)
  # Disable SPIR-V validation on Android because it adds a significant amount
  # to the binary size, and Tint's output should be well-formed.
  set(ENABLE_SPIRV_VALIDATION OFF)
elseif(UNIX)
  set(ENABLE_OPENGLES ON)
  set(ENABLE_DESKTOP_GL ON)
  set(ENABLE_VULKAN ON)
  set(ENABLE_SPIRV_VALIDATION ON)
  set(USE_X11 ON)
endif()

# GLFW is not supported in UWP
set(DAWN_SUPPORTS_GLFW_FOR_WINDOWING OFF)
if (EMSCRIPTEN OR (WIN32 AND NOT WINDOWS_STORE) OR (UNIX AND NOT ANDROID))
    set(DAWN_SUPPORTS_GLFW_FOR_WINDOWING ON)
endif()

# Current examples are depend on GLFW
if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    set(BUILD_SAMPLES ON)
endif()

option(DAWN_ENABLE_ASAN "Enable address sanitizer" OFF)
option(DAWN_ENABLE_INSTALL "Enable install step for Dawn libraries" OFF)
option(DAWN_ENABLE_TSAN "Enable thread sanitizer" OFF)
option(DAWN_ENABLE_MSAN "Enable memory sanitizer" OFF)
option(DAWN_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)

option(DAWN_ENABLE_D3D11 "Enable compilation of the D3D11 backend" ${ENABLE_D3D11})
option(DAWN_ENABLE_D3D12 "Enable compilation of the D3D12 backend" ${ENABLE_D3D12})
option(DAWN_ENABLE_METAL "Enable compilation of the Metal backend" ${ENABLE_METAL})
option(DAWN_ENABLE_NULL "Enable compilation of the Null backend" ${ENABLE_NULL})
option(DAWN_ENABLE_WEBGPU_ON_WEBGPU "Enable compilation of the WebGPU backend" ${ENABLE_WEBGPU_ON_WEBGPU})
option(DAWN_ENABLE_DESKTOP_GL "Enable compilation of the OpenGL backend" ${ENABLE_DESKTOP_GL})
option(DAWN_ENABLE_OPENGLES "Enable compilation of the OpenGL ES backend" ${ENABLE_OPENGLES})
option(DAWN_ENABLE_VULKAN "Enable compilation of the Vulkan backend" ${ENABLE_VULKAN})
option(DAWN_ENABLE_SPIRV_VALIDATION "Enable validation of SPIR-V" ${ENABLE_SPIRV_VALIDATION})
option(DAWN_FORCE_SYSTEM_COMPONENT_LOAD "Allow system component fallback" ${FORCE_SYSTEM_COMPONENT_LOAD})

message(STATUS "Dawn building using Emscripten toolchain: ${EMSCRIPTEN}")
message(STATUS "Dawn build D3D11 backend: ${DAWN_ENABLE_D3D11}")
message(STATUS "Dawn build D3D12 backend: ${DAWN_ENABLE_D3D12}")
message(STATUS "Dawn build Metal backend: ${DAWN_ENABLE_METAL}")
message(STATUS "Dawn build Vulkan backend: ${DAWN_ENABLE_VULKAN}")
message(STATUS "Dawn build OpenGL backend: ${DAWN_ENABLE_DESKTOP_GL}")
message(STATUS "Dawn build OpenGL ES backend: ${DAWN_ENABLE_OPENGLES}")
message(STATUS "Dawn build Null backend: ${DAWN_ENABLE_NULL}")
message(STATUS "")
message(STATUS "Dawn enable SPIR-V validation: ${DAWN_ENABLE_SPIRV_VALIDATION}")
message(STATUS "")
message(STATUS "Dawn build with ASAN: ${DAWN_ENABLE_ASAN}")
message(STATUS "Dawn build with TSAN: ${DAWN_ENABLE_TSAN}")
message(STATUS "Dawn build with MSAN: ${DAWN_ENABLE_MSAN}")
message(STATUS "Dawn build with UBSAN: ${DAWN_ENABLE_UBSAN}")
message(STATUS "Dawn enable install: ${DAWN_ENABLE_INSTALL}")
message(STATUS "Dawn allow system component fallback: ${DAWN_FORCE_SYSTEM_COMPONENT_LOAD}")
message(STATUS "")

option(DAWN_ALWAYS_ASSERT "Enable assertions on all build types" OFF)
option(DAWN_USE_WAYLAND "Enable support for Wayland surface" ${USE_WAYLAND})
option(DAWN_USE_X11 "Enable support for X11 surface" ${USE_X11})
option(DAWN_USE_GLFW "Enable compilation of the GLFW windowing utils" ${DAWN_SUPPORTS_GLFW_FOR_WINDOWING})
option(DAWN_USE_WINDOWS_UI "Enable support for Windows UI surface" ${USE_WINDOWS_UI})
option(DAWN_USE_BUILT_DXC "Enable building and using DXC by the D3D12 backend" OFF)
option(DAWN_DXC_ENABLE_ASSERTS_IN_NDEBUG "Enable DXC asserts in non-debug builds" ON)
option(DAWN_TARGET_MACOS "Manually link Apple core frameworks" ${TARGET_MACOS})

option(DAWN_BUILD_SAMPLES "Enables building Dawn's samples" ${BUILD_SAMPLES})
option(DAWN_BUILD_TESTS "Enables building Dawn's tests" ${BUILD_TESTS})
option(DAWN_BUILD_NODE_BINDINGS "Enables building Dawn's NodeJS bindings" OFF)
option(DAWN_ENABLE_SWIFTSHADER "Enables building Swiftshader as part of the build and Vulkan adapter discovery" OFF)
option(DAWN_BUILD_BENCHMARKS "Build Dawn benchmarks" OFF)
option(DAWN_BUILD_PROTOBUF "Build the protobuf dependencies" ON)

option(DAWN_WERROR "Build with -Werror (or equivalent)" OFF)
option(DAWN_WEVERYTHING "Build with -Weverything if using Clang" OFF)
option(DAWN_ENABLE_PIC "Build with Position-Independent-Code enabled" OFF)

option(DAWN_EMIT_COVERAGE "Emit code coverage information" OFF)
set_if_not_defined(LLVM_SOURCE_DIR "${Dawn_LLVM_SOURCE_DIR}" "Directory to an LLVM source checkout. Required to build turbo-cov")

message(STATUS "Dawn build with asserts in all configurations: ${DAWN_ALWAYS_ASSERT}")
message(STATUS "Dawn build Wayland support: ${DAWN_USE_WAYLAND}")
message(STATUS "Dawn build X11 support: ${DAWN_USE_X11}")
message(STATUS "Dawn build GLFW support: ${DAWN_USE_GLFW}")
message(STATUS "Dawn build Windows UI support: ${DAWN_USE_WINDOWS_UI}")
message(STATUS "Dawn build and use DXC: ${DAWN_USE_BUILT_DXC}")
message(STATUS "Dawn enable DXC asserts in non-debug builds: ${DAWN_DXC_ENABLE_ASSERTS_IN_NDEBUG}")
message(STATUS "Dawn target MacOS: ${TARGET_MACOS}")
message(STATUS "")
message(STATUS "Dawn build samples: ${DAWN_BUILD_SAMPLES}")
message(STATUS "Dawn build Node bindings: ${DAWN_BUILD_NODE_BINDINGS}")
message(STATUS "Dawn build Swiftshader: ${DAWN_ENABLE_SWIFTSHADER}")
message(STATUS "Dawn build benchmarks: ${DAWN_BUILD_BENCHMARKS}")
message(STATUS "Dawn build protobuf: ${DAWN_BUILD_PROTOBUF}")
message(STATUS "")
message(STATUS "Dawn build PIC: ${DAWN_ENABLE_PIC}")
message(STATUS "DAWN Werror: ${DAWN_WERROR}")
message(STATUS "")
message(STATUS "Dawn emit coverage: ${DAWN_EMIT_COVERAGE}")
message(STATUS "LLVM Source dir: ${LLVM_SOURCE_DIR}")
message(STATUS "")

if (DAWN_ENABLE_OPENGLES OR DAWN_ENABLE_DESKTOP_GL)
  set(TINT_DEFAULT_GLSL ON)
else()
  set(TINT_DEFAULT_GLSL OFF)
endif()

option(TINT_ENABLE_INSTALL "Enable install step for Tint libraries" OFF)
option(TINT_BUILD_CMD_TOOLS "Build the Tint command line tools" ON)

if (DAWN_BUILD_SAMPLES)
  if (NOT DAWN_USE_GLFW AND NOT EMSCRIPTEN)
    message(SEND_ERROR "Dawn samples require GLFW or Emscripten")
  endif()
endif()

option(TINT_BUILD_SPV_READER "Build the SPIR-V input reader" ${DAWN_ENABLE_VULKAN})
option(TINT_BUILD_WGSL_READER "Build the WGSL input reader" ON)
option(TINT_BUILD_GLSL_WRITER "Build the GLSL output writer" ${TINT_DEFAULT_GLSL})
option(TINT_BUILD_GLSL_VALIDATOR "Build the GLSL output validator" ON)
option(TINT_BUILD_HLSL_WRITER "Build the HLSL output writer" ${DAWN_ENABLE_D3D12})
option(TINT_BUILD_MSL_WRITER "Build the MSL output writer" ${DAWN_ENABLE_METAL})
option(TINT_BUILD_SPV_WRITER "Build the SPIR-V output writer" ${DAWN_ENABLE_VULKAN})
option(TINT_BUILD_WGSL_WRITER "Build the WGSL output writer" ON)
option(TINT_BUILD_SYNTAX_TREE_WRITER "Build the syntax tree writer" OFF)

option(TINT_BUILD_IR_BINARY "Build IR binary format support" ${DAWN_BUILD_PROTOBUF})
option(TINT_BUILD_FUZZERS "Build fuzzers" OFF)
option(TINT_BUILD_BENCHMARKS "Build Tint benchmarks" OFF)
option(TINT_BUILD_TESTS "Build tests" ON)
option(TINT_BUILD_AS_OTHER_OS "Override OS detection to force building of *_other.cc files" OFF)

if (EMSCRIPTEN)
  # Skip tint tests in emscripten build
  set(TINT_BUILD_TESTS OFF)
  # Skip GLSL validation in Emscripten
  set(TINT_BUILD_GLSL_VALIDATOR OFF)
endif()

option(TINT_BUILD_TINTD "Build the WGSL language server" OFF)

option(TINT_ENABLE_IR_VALIDATION "Enable IR validation for backend codegen" ${IS_DEBUG_BUILD})
option(TINT_ENABLE_BREAK_IN_DEBUGGER "Enable tint::debugger::Break()" OFF)
option(TINT_CHECK_CHROMIUM_STYLE "Check for [chromium-style] issues during build" OFF)
option(TINT_RANDOMIZE_HASHES "Randomize the hash seed value to detect non-deterministic output" OFF)

message(STATUS "Tint build SPIR-V reader: ${TINT_BUILD_SPV_READER}")
message(STATUS "Tint build WGSL reader: ${TINT_BUILD_WGSL_READER}")
message(STATUS "Tint build GLSL writer: ${TINT_BUILD_GLSL_WRITER}")
message(STATUS "Tint build GLSL validator: ${TINT_BUILD_GLSL_VALIDATOR}")
message(STATUS "Tint build HLSL writer: ${TINT_BUILD_HLSL_WRITER}")
message(STATUS "Tint build MSL writer: ${TINT_BUILD_MSL_WRITER}")
message(STATUS "Tint build SPIR-V writer: ${TINT_BUILD_SPV_WRITER}")
message(STATUS "Tint build WGSL writer: ${TINT_BUILD_WGSL_WRITER}")
message(STATUS "Tint build Syntax Tree writer: ${TINT_BUILD_SYNTAX_TREE_WRITER}")
message(STATUS "")
message(STATUS "Tint build command line executable tools: ${TINT_BUILD_CMD_TOOLS}")
message(STATUS "Tint install: ${TINT_ENABLE_INSTALL}")
message(STATUS "Tint build fuzzers: ${TINT_BUILD_FUZZERS}")
message(STATUS "Tint build IR binary: ${TINT_BUILD_IR_BINARY}")
message(STATUS "Tint build benchmarks: ${TINT_BUILD_BENCHMARKS}")
message(STATUS "Tint build tests: ${TINT_BUILD_TESTS}")
message(STATUS "Tint build tintd: ${TINT_BUILD_TINTD}")
message(STATUS "")
message(STATUS "Tint enable IR validation: ${TINT_ENABLE_IR_VALIDATION}")
message(STATUS "Tint enable break in debugger: ${TINT_ENABLE_BREAK_IN_DEBUGGER}")
message(STATUS "Tint build checking [chromium-style]: ${TINT_CHECK_CHROMIUM_STYLE}")
message(STATUS "Tint randomize hashes: ${TINT_RANDOMIZE_HASHES}")
message(STATUS "")

set_if_not_defined(DAWN_THIRD_PARTY_DIR "${Dawn_SOURCE_DIR}/third_party" "Directory in which to find third-party dependencies.")

set_if_not_defined(DAWN_ABSEIL_DIR "${DAWN_THIRD_PARTY_DIR}/abseil-cpp" "Directory in which to find Abseil")
set_if_not_defined(DAWN_GLFW_DIR "${DAWN_THIRD_PARTY_DIR}/glfw" "Directory in which to find GLFW")
set_if_not_defined(DAWN_JINJA2_DIR "${DAWN_THIRD_PARTY_DIR}/jinja2" "Directory in which to find Jinja2")
set_if_not_defined(DAWN_MARKUPSAFE_DIR "${DAWN_THIRD_PARTY_DIR}/markupsafe" "Directory in which to find MarkupSafe")
set_if_not_defined(DAWN_KHRONOS_DIR "${DAWN_THIRD_PARTY_DIR}/khronos" "Directory in which to find Khronos GL headers")
set_if_not_defined(DAWN_SWIFTSHADER_DIR "${DAWN_THIRD_PARTY_DIR}/swiftshader" "Directory in which to find swiftshader")
set_if_not_defined(DAWN_PROTOBUF_DIR "${DAWN_THIRD_PARTY_DIR}/protobuf" "Directory in which to find protobuf")
set_if_not_defined(DAWN_LPM_DIR "${DAWN_THIRD_PARTY_DIR}/libprotobuf-mutator/src" "Directory in which to find libprotobuf")
set_if_not_defined(DAWN_EMDAWNWEBGPU_DIR "${DAWN_THIRD_PARTY_DIR}/emdawnwebgpu" "Directory in which to find Dawn specific Emscripten bindings")
set_if_not_defined(DAWN_GOOGLETEST_DIR "${DAWN_THIRD_PARTY_DIR}/googletest" "Directory in which to find googletest")

set_if_not_defined(DAWN_SPIRV_TOOLS_DIR "${DAWN_THIRD_PARTY_DIR}/spirv-tools/src" "Directory in which to find SPIRV-Tools")
set_if_not_defined(DAWN_SPIRV_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/spirv-headers/src" "Directory in which to find SPIRV-Headers")
set_if_not_defined(DAWN_GLSLANG_DIR "${DAWN_THIRD_PARTY_DIR}/glslang/src" "Directory in which to find GLSLang")
set_if_not_defined(DAWN_VULKAN_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-headers/src" "Directory in which to find Vulkan-Headers")
set_if_not_defined(DAWN_VULKAN_UTILITY_LIBRARIES_DIR "${DAWN_THIRD_PARTY_DIR}/vulkan-utility-libraries/src" "Directory in which to find Vulkan-Utility-Libraries")

message(STATUS "Dawn third_party dir: ${DAWN_THIRD_PARTY_DIR}")
message(STATUS "Dawn GLFW dir: ${DAWN_GLFW_DIR}")
message(STATUS "Dawn Jinja2 dir: ${DAWN_JINJA2_DIR}")
message(STATUS "Dawn MarkupSafe dir: ${DAWN_MARKUPSAFE_DIR}")
message(STATUS "Dawn Khronos dir: ${DAWN_KHRONOS_DIR}")
message(STATUS "Dawn Swiftshader dir: ${DAWN_SWIFTSHADER_DIR}")
message(STATUS "Dawn Protobuf dir: ${DAWN_PROTOBUF_DIR}")
message(STATUS "Dawn LPM dir: ${DAWN_LPM_DIR}")
message(STATUS "Dawn Emdawnwebgpu dir: ${DAWN_EMDAWNWEBGPU_DIR}")
message(STATUS "Dawn Spir-Tools dir: ${DAWN_SPIRV_TOOLS_DIR}")
message(STATUS "Dawn Spirv-Headers dir: ${DAWN_SPIRV_HEADERS_DIR}")
message(STATUS "Dawn Glslang dir: ${DAWN_GLSLANG_DIR}")
message(STATUS "Dawn Vulkan Headers dir: ${DAWN_VULKAN_HEADERS_DIR}")
message(STATUS "Dawn Vulkan Utility Libraries dir: ${DAWN_VULKAN_UTILITY_LIBRARIES_DIR}")
message(STATUS "")

# Dependencies for DAWN_BUILD_NODE_BINDINGS
set_if_not_defined(NODE_ADDON_API_DIR "${DAWN_THIRD_PARTY_DIR}/node-addon-api" "Directory in which to find node-addon-api")
set_if_not_defined(NODE_API_HEADERS_DIR "${DAWN_THIRD_PARTY_DIR}/node-api-headers" "Directory in which to find node-api-headers")
set_if_not_defined(WEBGPU_IDL_PATH "${DAWN_THIRD_PARTY_DIR}/gpuweb/webgpu.idl" "Path to the webgpu.idl definition file")
set_if_not_defined(GO_EXECUTABLE "go" "Golang executable for running the IDL generator")

message(STATUS "Node Addon API dir: ${NODE_ADDON_API_DIR}")
message(STATUS "Node API Headers dir: ${NODE_API_HEADERS_DIR}")
message(STATUS "Webgpu IDL path: ${WEBGPU_IDL_PATH}")
message(STATUS "Go exe: ${GO_EXECUTABLE}")
message(STATUS "")

# Handling of the type of monolithic library to build, if any.
set(DAWN_BUILD_MONOLITHIC_LIBRARY "OFF" CACHE STRING "Build monolithic library: SHARED, STATIC, or OFF.")
set_property(CACHE DAWN_BUILD_MONOLITHIC_LIBRARY PROPERTY STRINGS SHARED SHARED STATIC)

string(TOUPPER "${DAWN_BUILD_MONOLITHIC_LIBRARY}" _option_upper)
if(NOT _option_upper STREQUAL "OFF" AND
   NOT _option_upper STREQUAL "SHARED" AND
   NOT _option_upper STREQUAL "STATIC")
    message(FATAL_ERROR "DAWN_BUILD_MONOLITHIC_LIBRARY must be SHARED, STATIC, or OFF, but was \"${DAWN_BUILD_MONOLITHIC_LIBRARY}\".")
endif()
if (DAWN_BUILD_MONOLITHIC_LIBRARY AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "DAWN_BUILD_MONOLITHIC_LIBRARY SHARED/STATIC requires BUILD_SHARED_LIBS=OFF. Otherwise the bundled library will depend on other shared libraries which defeats the purpose.")
endif()
message(STATUS "Dawn build monolithic library: ${DAWN_BUILD_MONOLITHIC_LIBRARY}")

option(DAWN_FETCH_DEPENDENCIES "Use fetch_dawn_dependencies.py as an alternative to using depot_tools" OFF)
message(STATUS "Dawn fetch dependencies: ${DAWN_FETCH_DEPENDENCIES}")
message(STATUS "")

# Much of the backend code is shared among desktop OpenGL and OpenGL ES
if (${DAWN_ENABLE_DESKTOP_GL} OR ${DAWN_ENABLE_OPENGLES})
    set(DAWN_ENABLE_OPENGL ON)
endif()

if(DAWN_ENABLE_PIC)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Defines `CMAKE_INSTALL_*DIR` variables
include(GNUInstallDirs)
# The public config contains only the include paths for the Dawn headers.
add_library(dawn_public_config INTERFACE)
target_include_directories(dawn_public_config INTERFACE
    "$<BUILD_INTERFACE:${DAWN_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${DAWN_BUILD_GEN_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# The internal config contains additional path but includes the dawn_public_config include paths
add_library(dawn_internal_config INTERFACE)
target_include_directories(dawn_internal_config INTERFACE
    "${PROJECT_SOURCE_DIR}"
    "${DAWN_SRC_DIR}"
    "${DAWN_BUILD_GEN_DIR}/src"
)
target_link_libraries(dawn_internal_config INTERFACE dawn_public_config)

################################################################################
# Include utility CMake modules
################################################################################
include(DawnCompilerChecks)
include(DawnCompilerExtraFlags)
include(DawnCompilerPlatformFlags)
include(DawnCompilerWarningFlags)
include(DawnInitializeBuildType)
include(DawnLibrary)

if (NOT ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS} STREQUAL "")
  message(STATUS "Using provided LIB_FUZZING_ENGINE options: ${TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS}")
endif()

find_package(Python3 REQUIRED)
message(STATUS "Dawn: using python at ${Python3_EXECUTABLE}")

# Options and compile definitions for the internal config
add_dependencies(dawn_internal_config dawn_warnings_config)
if (DAWN_ALWAYS_ASSERT OR IS_DEBUG_BUILD)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_ASSERTS")
endif()
if (DAWN_ENABLE_D3D11)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_D3D11")
endif()
if (DAWN_ENABLE_D3D12)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_D3D12")
endif()
if (DAWN_ENABLE_METAL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_METAL")
endif()
if (DAWN_ENABLE_NULL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_NULL")
endif()
if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_WEBGPU")
endif()
if (DAWN_ENABLE_DESKTOP_GL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_DESKTOP_GL")
endif()
if (DAWN_ENABLE_OPENGLES)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_OPENGLES")
endif()
if (DAWN_ENABLE_OPENGL)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_OPENGL")
endif()
if (DAWN_ENABLE_VULKAN)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_ENABLE_BACKEND_VULKAN")
endif()
if (DAWN_USE_WAYLAND)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_WAYLAND")
endif()
if (DAWN_USE_X11)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_X11")
endif()
if (DAWN_USE_WINDOWS_UI)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_USE_WINDOWS_UI")
endif()
if (WIN32)
    target_compile_definitions(dawn_internal_config INTERFACE "NOMINMAX" "WIN32_LEAN_AND_MEAN")
endif()
if (DAWN_FORCE_SYSTEM_COMPONENT_LOAD)
    target_compile_definitions(dawn_internal_config INTERFACE "DAWN_FORCE_SYSTEM_COMPONENT_LOAD")
endif()

################################################################################
# Tint
################################################################################

set(TINT_LIB_FUZZING_ENGINE_LINK_OPTIONS "" CACHE STRING "Used by OSS-Fuzz to control, via link options, which fuzzing engine should be used")

set(TINT_ROOT_SOURCE_DIR   ${PROJECT_SOURCE_DIR})
set(TINT_SPIRV_HEADERS_DIR ${DAWN_SPIRV_HEADERS_DIR})
set(TINT_SPIRV_TOOLS_DIR   ${DAWN_SPIRV_TOOLS_DIR})


################################################################################
# Run on all subdirectories
################################################################################
add_subdirectory(third_party)

# TODO(crbug.com/tint/455): Tint does not currently build with CMake when
# BUILD_SHARED_LIBS=1, so always build it as static for now.
set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS 0)

add_subdirectory(src/utils)
add_subdirectory(src/tint)

set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED})

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12 OR DAWN_ENABLE_METAL OR DAWN_ENABLE_NULL OR DAWN_ENABLE_DESKTOP_GL OR DAWN_ENABLE_OPENGLES OR DAWN_ENABLE_VULKAN OR EMSCRIPTEN)
    add_subdirectory(generator)
    # Note that we must add Emscripten directory first to ensure the correct
    # headers will be exported upwards.
    add_subdirectory(src/emdawnwebgpu)
    add_subdirectory(src/dawn)
endif()

################################################################################
# Samples
################################################################################
add_custom_target(tint-lint
  COMMAND ./tools/lint
  WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
  COMMENT "Running linter"
  VERBATIM)

add_custom_target(tint-format
  COMMAND ./tools/format
  WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
  COMMENT "Running formatter"
  VERBATIM)

if (DAWN_EMIT_COVERAGE)
  add_subdirectory(tools/src/cmd/turbo-cov)

  # The tint-generate-coverage target generates a lcov.info file at the project
  # root, holding the code coverage for all the tint_unitests.
  # This can be used by tools such as VSCode's Coverage Gutters extension to
  # visualize code coverage in the editor.
  get_filename_component(CLANG_BIN_DIR ${CMAKE_C_COMPILER} DIRECTORY)
  set(PATH_WITH_CLANG "${CLANG_BIN_DIR}:$ENV{PATH}")
  add_custom_target(tint-generate-coverage
    COMMAND ${CMAKE_COMMAND} -E env PATH=${PATH_WITH_CLANG} ./tools/tint-generate-coverage $<TARGET_FILE:tint_unittests>
    DEPENDS tint_unittests
    WORKING_DIRECTORY ${TINT_ROOT_SOURCE_DIR}
    COMMENT "Generating tint coverage data"
    VERBATIM)
endif()

if (DAWN_BUILD_MONOLITHIC_LIBRARY AND DAWN_ENABLE_INSTALL)
  # This series of functions add the necessary information so that other CMake projects
  # can use Dawn, be it from a build directory, a local install or when packaged.
  install(EXPORT DawnTargets
    FILE DawnTargets.cmake
    NAMESPACE dawn::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dawn")
  export(EXPORT DawnTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/DawnTargets.cmake"
      NAMESPACE dawn::
  )
  # Create a ConfigVersion.cmake file:
  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DawnConfigVersion.cmake"
    COMPATIBILITY AnyNewerVersion)
  # Configure config file
  configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/src/cmake/DawnConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DawnConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dawn"
  )
  # Install the fully generated config and configVersion files
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/DawnConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/DawnConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dawn"
  )
endif ()
