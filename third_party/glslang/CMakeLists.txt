# Copyright (C) 2020 The Khronos Group Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#
#    Neither the name of The Khronos Group Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

project(glslang
    LANGUAGES CXX)

# increase to 3.1 once all major distributions
# include a version of CMake >= 3.1
cmake_minimum_required(VERSION 2.8.12)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable compile commands database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)

# Needed for CMAKE_DEPENDENT_OPTION macro
include(CMakeDependentOption)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(BUILD_EXTERNAL "Build external dependencies in /External" ON)

set(LIB_TYPE STATIC)

if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    # This logic inside SPIRV-Tools, which can upset build target dependencies
    # if changed after targets are already defined. To prevent these issues,
    # ensure CMAKE_BUILD_TYPE is assigned early and at the glslang root scope.
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

option(SKIP_GLSLANG_INSTALL "Skip installation" ${SKIP_GLSLANG_INSTALL})
if(NOT ${SKIP_GLSLANG_INSTALL})
  set(ENABLE_GLSLANG_INSTALL ON)
endif()
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" ON)

option(ENABLE_GLSLANG_BINARIES "Builds glslangValidator and spirv-remap" ON)

option(ENABLE_GLSLANG_JS
    "If using Emscripten, build glslang.js. Otherwise, builds a sample executable for binary-size testing." OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_GLSLANG_WEBMIN
    "Reduces glslang to minimum needed for web use"
    OFF "ENABLE_GLSLANG_JS"
    OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_GLSLANG_WEBMIN_DEVEL
    "For ENABLE_GLSLANG_WEBMIN builds, enables compilation error messages"
    OFF "ENABLE_GLSLANG_WEBMIN"
    OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_EMSCRIPTEN_SINGLE_FILE
    "If using Emscripten, enables SINGLE_FILE build"
    OFF "ENABLE_GLSLANG_JS AND EMSCRIPTEN"
    OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_EMSCRIPTEN_ENVIRONMENT_NODE
    "If using Emscripten, builds to run on Node instead of Web"
    OFF "ENABLE_GLSLANG_JS AND EMSCRIPTEN"
    OFF)

CMAKE_DEPENDENT_OPTION(ENABLE_HLSL
    "Enables HLSL input support"
    ON "NOT ENABLE_GLSLANG_WEBMIN"
    OFF)

option(ENABLE_RTTI "Enables RTTI" OFF)
option(ENABLE_EXCEPTIONS "Enables Exceptions" OFF)
option(ENABLE_OPT "Enables spirv-opt capability if present" ON)
option(ENABLE_PCH "Enables Precompiled header" ON)
option(ENABLE_CTEST "Enables testing" ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND WIN32)
    set(CMAKE_INSTALL_PREFIX "install" CACHE STRING "..." FORCE)
endif()

option(USE_CCACHE "Use ccache" OFF)
if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    endif(CCACHE_FOUND)
endif()

if(ENABLE_CTEST)
    include(CTest)
endif()

if(ENABLE_HLSL)
    add_definitions(-DENABLE_HLSL)
endif(ENABLE_HLSL)

if(ENABLE_GLSLANG_WEBMIN)
    add_definitions(-DGLSLANG_WEB)
    if(ENABLE_GLSLANG_WEBMIN_DEVEL)
        add_definitions(-DGLSLANG_WEB_DEVEL)
    endif(ENABLE_GLSLANG_WEBMIN_DEVEL)
endif(ENABLE_GLSLANG_WEBMIN)

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
    if(MSVC)
        include(ChooseMSVCCRT.cmake)
    endif(MSVC)
    add_definitions(-DGLSLANG_OSINCLUDE_WIN32)
elseif(UNIX)
    add_definitions(-DGLSLANG_OSINCLUDE_UNIX)
else(WIN32)
    message("unknown platform")
endif(WIN32)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-Wall -Wmaybe-uninitialized -Wuninitialized -Wunused -Wunused-local-typedefs
                        -Wunused-parameter -Wunused-value  -Wunused-variable -Wunused-but-set-parameter -Wunused-but-set-variable -fno-exceptions)
    add_compile_options(-Wno-reorder)  # disable this from -Wall, since it happens all over.
    if(NOT ENABLE_RTTI)
        add_compile_options(-fno-rtti)
    endif()
    if(NOT ENABLE_EXCEPTIONS)
        add_compile_options(-fno-exceptions)
    endif()
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")
        add_compile_options(-Werror=deprecated-copy)
    endif()

    if(NOT CMAKE_VERSION VERSION_LESS "3.13")
        # Error if there's symbols that are not found at link time.
        # add_link_options() was added in CMake 3.13 - if using an earlier
        # version don't set this - it should be caught by presubmits anyway.
        add_link_options("-Wl,--no-undefined")
    endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT MSVC)
    add_compile_options(-Wall -Wuninitialized -Wunused -Wunused-local-typedefs
                        -Wunused-parameter -Wunused-value  -Wunused-variable)
    add_compile_options(-Wno-reorder)  # disable this from -Wall, since it happens all over.
    if(NOT ENABLE_RTTI)
        add_compile_options(-fno-rtti)
    endif()
    if(NOT ENABLE_EXCEPTIONS)
        add_compile_options(-fno-exceptions)
    endif()

    if(NOT CMAKE_VERSION VERSION_LESS "3.13")
        # Error if there's symbols that are not found at link time.
        # add_link_options() was added in CMake 3.13 - if using an earlier
        # version don't set this - it should be caught by presubmits anyway.
        add_link_options("-Wl,-undefined,error")
    endif()
elseif(MSVC)
    if(NOT ENABLE_RTTI)
        string(FIND "${CMAKE_CXX_FLAGS}" "/GR" MSVC_HAS_GR)
        if(MSVC_HAS_GR)
            string(REGEX REPLACE /GR /GR- CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        else()
            add_compile_options(/GR-) # Disable RTTI
        endif()
    endif()
    if(ENABLE_EXCEPTIONS)
        add_compile_options(/EHsc) # Enable Exceptions
    endif()
endif()

if(ENABLE_GLSLANG_JS)
    if(MSVC)
        add_compile_options(/Os /GR-)
    else()
        add_compile_options(-Os -fno-rtti -fno-exceptions)
        if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT MSVC)
            add_compile_options(-Wno-unused-parameter)
            add_compile_options(-Wno-unused-variable -Wno-unused-const-variable)
        endif()
    endif()
endif(ENABLE_GLSLANG_JS)

# Request C++11
if(${CMAKE_VERSION} VERSION_LESS 3.1)
    # CMake versions before 3.1 do not understand CMAKE_CXX_STANDARD
    # remove this block once CMake >=3.1 has fixated in the ecosystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

function(glslang_set_link_args TARGET)
    # For MinGW compiles, statically link against the GCC and C++ runtimes.
    # This avoids the need to ship those runtimes as DLLs.
    if(WIN32 AND ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
        set_target_properties(${TARGET} PROPERTIES
                              LINK_FLAGS "-static -static-libgcc -static-libstdc++")
    endif()
endfunction(glslang_set_link_args)

# CMake needs to find the right version of python, right from the beginning,
# otherwise, it will find the wrong version and fail later
find_package(PythonInterp 3 REQUIRED)

# Root directory for build-time generated include files
set(GLSLANG_GENERATED_INCLUDEDIR "${CMAKE_BINARY_DIR}/include")

################################################################################
# Build version information generation
################################################################################
set(GLSLANG_CHANGES_FILE      "${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md")
set(GLSLANG_BUILD_INFO_PY     "${CMAKE_CURRENT_SOURCE_DIR}/build_info.py")
set(GLSLANG_BUILD_INFO_H_TMPL "${CMAKE_CURRENT_SOURCE_DIR}/build_info.h.tmpl")
set(GLSLANG_BUILD_INFO_H      "${GLSLANG_GENERATED_INCLUDEDIR}/glslang/build_info.h")

# Command to build the build_info.h file
add_custom_command(
    OUTPUT  ${GLSLANG_BUILD_INFO_H}
    COMMAND ${PYTHON_EXECUTABLE} "${GLSLANG_BUILD_INFO_PY}"
            ${CMAKE_CURRENT_SOURCE_DIR}
            "-i" ${GLSLANG_BUILD_INFO_H_TMPL}
            "-o" ${GLSLANG_BUILD_INFO_H}
    DEPENDS ${GLSLANG_BUILD_INFO_PY}
            ${GLSLANG_CHANGES_FILE}
            ${GLSLANG_BUILD_INFO_H_TMPL}
    COMMENT "Generating ${GLSLANG_BUILD_INFO_H}")

# Target to build the build_info.h file
add_custom_target(glslang-build-info DEPENDS ${GLSLANG_BUILD_INFO_H})

# Populate the CMake GLSLANG_VERSION* variables with the build version
# information.
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "${GLSLANG_BUILD_INFO_PY}"
            ${CMAKE_CURRENT_SOURCE_DIR} "<major>.<minor>.<patch><-flavor>;<major>;<minor>;<patch>;<flavor>"
    OUTPUT_VARIABLE "GLSLANG_VERSIONS"
    OUTPUT_STRIP_TRAILING_WHITESPACE)
list(GET "GLSLANG_VERSIONS" 0 "GLSLANG_VERSION")
list(GET "GLSLANG_VERSIONS" 1 "GLSLANG_VERSION_MAJOR")
list(GET "GLSLANG_VERSIONS" 2 "GLSLANG_VERSION_MINOR")
list(GET "GLSLANG_VERSIONS" 3 "GLSLANG_VERSION_PATCH")
list(GET "GLSLANG_VERSIONS" 4 "GLSLANG_VERSION_FLAVOR")
configure_file(${GLSLANG_CHANGES_FILE} "${CMAKE_CURRENT_BINARY_DIR}/CHANGES.md") # Required to re-run cmake on version change

# glslang_add_build_info_dependency() adds the glslang-build-info dependency and
# generated include directories to target.
function(glslang_add_build_info_dependency target)
    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${GLSLANG_GENERATED_INCLUDEDIR}>)
    add_dependencies(${target} glslang-build-info)
endfunction()

# glslang_only_export_explicit_symbols() makes the symbol visibility hidden by
# default for <target> when building shared libraries, and sets the
# GLSLANG_IS_SHARED_LIBRARY define, and GLSLANG_EXPORTING to 1 when specifically
# building <target>.
function(glslang_only_export_explicit_symbols target)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${target} PUBLIC "GLSLANG_IS_SHARED_LIBRARY=1")
        if(WIN32)
            target_compile_definitions(${target} PRIVATE "GLSLANG_EXPORTING=1")
        else()
            target_compile_options(${target} PRIVATE "-fvisibility=hidden")
        endif()
    endif()
endfunction()

# glslang_pch() adds precompiled header rules to <target> for the pre-compiled
# header file <pch>. As target_precompile_headers() was added in CMake 3.16,
# this is a no-op if called on earlier versions of CMake.
if(NOT CMAKE_VERSION VERSION_LESS "3.16")
    function(glslang_pch target pch)
        target_precompile_headers(${target} PRIVATE ${pch})
    endfunction()
else()
    function(glslang_pch target pch)
    endfunction()
    message("Your CMake version is ${CMAKE_VERSION}. Update to at least 3.16 to enable precompiled headers to speed up incremental builds")
endif()

if(BUILD_EXTERNAL AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External)
    # We depend on these for later projects, so they should come first.
    add_subdirectory(External)
endif()

if(NOT TARGET SPIRV-Tools-opt)
    set(ENABLE_OPT OFF)
endif()

if(ENABLE_OPT)
    message(STATUS "optimizer enabled")
    add_definitions(-DENABLE_OPT=1)
else()
    if(ENABLE_HLSL)
        message(STATUS "spirv-tools not linked - illegal SPIRV may be generated for HLSL")
    endif()
    add_definitions(-DENABLE_OPT=0)
endif()

add_subdirectory(glslang)
add_subdirectory(OGLCompilersDLL)
if(ENABLE_GLSLANG_BINARIES)
    add_subdirectory(StandAlone)
endif()
add_subdirectory(SPIRV)
if(ENABLE_HLSL)
    add_subdirectory(hlsl)
endif(ENABLE_HLSL)
if(ENABLE_CTEST)
    add_subdirectory(gtests)
endif()

if(ENABLE_CTEST AND BUILD_TESTING)
    # glslang-testsuite runs a bash script on Windows.
    # Make sure to use '-o igncr' flag to ignore carriage returns (\r).
    set(IGNORE_CR_FLAG "")
    if(WIN32)
        set(IGNORE_CR_FLAG -o igncr)
    endif()

    if (CMAKE_CONFIGURATION_TYPES)
        set(RESULTS_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/localResults)
        set(VALIDATOR_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/$<CONFIGURATION>/glslangValidator)
        set(REMAP_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/$<CONFIGURATION>/spirv-remap)
    else(CMAKE_CONFIGURATION_TYPES)
        set(RESULTS_PATH ${CMAKE_CURRENT_BINARY_DIR}/localResults)
        set(VALIDATOR_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/glslangValidator)
        set(REMAP_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/spirv-remap)
    endif(CMAKE_CONFIGURATION_TYPES)

    add_test(NAME glslang-testsuite
        COMMAND bash ${IGNORE_CR_FLAG} runtests ${RESULTS_PATH} ${VALIDATOR_PATH} ${REMAP_PATH}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Test/)
endif()
