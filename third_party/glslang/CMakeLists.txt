# Copyright (C) 2020-2025 The Khronos Group Inc.
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
cmake_minimum_required(VERSION 3.22.1)
project(glslang)

set(GLSLANG_TESTS_DEFAULT ON) # Can be turned off, below, based on environment.
set(GLSLANG_ENABLE_INSTALL_DEFAULT ON) # Can be turned off, below, based on environment.

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Can be removed when min is 3.26 see policy CMP0143.

# Adhere to GNU filesystem layout conventions
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Needed for cmake_dependent_option macro
include(CMakeDependentOption)

option(BUILD_SHARED_LIBS "Build Shared Libraries")
option(BUILD_EXTERNAL "Build external dependencies in /External" ON)
option(BUILD_WERROR "Enable warnings as errors (default is OFF)" OFF)

set(LIB_TYPE STATIC)

if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
endif()

get_cmake_property(isMultiConfig "GENERATOR_IS_MULTI_CONFIG")
if (NOT isMultiConfig AND NOT CMAKE_BUILD_TYPE)
    # This logic inside SPIRV-Tools, which can upset build target dependencies
    # if changed after targets are already defined. To prevent these issues,
    # ensure CMAKE_BUILD_TYPE is assigned early and at the glslang root scope.
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Currently iOS and Android are very similar.
# They both have their own packaging (APP/APK).
# Which makes regular executables/testing problematic.
#
# Currently the only deliverables for these platforms are
# libraries (either STATIC or SHARED).
#
# Furthermore testing is equally problematic.
if (IOS OR ANDROID)
    set(ENABLE_GLSLANG_BINARIES OFF)
    set(GLSLANG_TESTS_DEFAULT OFF)
endif()

# Simplify the default case of including this project.
# Otherwise add_subdirectory users have a harder time consuming the library.
# Since glslang will pollute the installation and add undesirable testing.
if(NOT PROJECT_IS_TOP_LEVEL)
    set(GLSLANG_TESTS_DEFAULT OFF)
    set(GLSLANG_ENABLE_INSTALL_DEFAULT OFF)
endif()

# Control whether Glslang self-tests are built and tested.
# Always expose this as an option, so the defaults can be overridden.
option(GLSLANG_TESTS "Enable glslang testing" ${GLSLANG_TESTS_DEFAULT})

# Control whether to install Glslang.
# Always expose this as an option, so the defaults can be overridden.
option(GLSLANG_ENABLE_INSTALL "Enable glslang installation" ${GLSLANG_ENABLE_INSTALL_DEFAULT})

option(ENABLE_SPIRV "Enables SPIRV output support" ON)
option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" ON)

option(ENABLE_GLSLANG_BINARIES "Builds glslang and spirv-remap" ON)

option(ENABLE_GLSLANG_JS "If using Emscripten, build glslang.js. Otherwise, builds a sample executable for binary-size testing.")
cmake_dependent_option(ENABLE_EMSCRIPTEN_SINGLE_FILE
    "If using Emscripten, enables SINGLE_FILE build"
    OFF "ENABLE_GLSLANG_JS AND EMSCRIPTEN"
    OFF)
cmake_dependent_option(ENABLE_EMSCRIPTEN_ENVIRONMENT_NODE
    "If using Emscripten, builds to run on Node instead of Web"
    OFF "ENABLE_GLSLANG_JS AND EMSCRIPTEN"
    OFF)

option(ENABLE_HLSL "Enables HLSL input support" ON)
option(ENABLE_RTTI "Enables RTTI")
option(ENABLE_EXCEPTIONS "Enables Exceptions")
cmake_dependent_option(ENABLE_OPT "Enables spirv-opt capability if present" ON "ENABLE_SPIRV" OFF)

if(MINGW OR (APPLE AND ${CMAKE_CXX_COMPILER_ID} MATCHES "GNU"))
    # Workaround for CMake behavior on Mac OS with gcc, cmake generates -Xarch_* arguments
    # which gcc rejects
    set(ENABLE_PCH OFF)
    message(NOTICE "Disabling PCH")
endif()

option(ENABLE_PCH "Enables Precompiled header" ON)

if(ENABLE_SPIRV)
    add_compile_definitions(ENABLE_SPIRV)
endif()

if(ENABLE_HLSL)
    add_compile_definitions(ENABLE_HLSL)
endif()

if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_compile_definitions(GLSLANG_OSINCLUDE_WIN32)
elseif(UNIX OR ANDROID)
    add_compile_definitions(GLSLANG_OSINCLUDE_UNIX)
else()
    message("unknown platform")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    add_compile_options(-Wall -Wmaybe-uninitialized -Wuninitialized -Wunused -Wunused-local-typedefs -Wimplicit-fallthrough
                        -Wunused-parameter -Wunused-value  -Wunused-variable -Wunused-but-set-parameter -Wunused-but-set-variable -fno-exceptions)
    if(NOT ENABLE_RTTI)
        add_compile_options(-fno-rtti)
    endif()
    if(NOT ENABLE_EXCEPTIONS)
        add_compile_options(-fno-exceptions)
    endif()
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0.0")
        add_compile_options(-Werror=deprecated-copy)
    endif()

    if(NOT (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD"))
        if (NOT APPLE)
            # Error if there's symbols that are not found at link time.
            add_link_options("-Wl,--no-undefined")
        endif()
    endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND NOT MSVC)
    add_compile_options(-Wall -Wuninitialized -Wunused -Wunused-local-typedefs -Wimplicit-fallthrough
                        -Wunused-parameter -Wunused-value  -Wunused-variable)
    if(NOT ENABLE_RTTI)
        add_compile_options(-fno-rtti)
    endif()
    if(NOT ENABLE_EXCEPTIONS)
        add_compile_options(-fno-exceptions)
    endif()

    if(NOT (CMAKE_SYSTEM_NAME MATCHES "OpenBSD|Emscripten"))
        # Error if there's symbols that are not found at link time. Some linkers do not support this flag.
        if(NOT APPLE)
            add_link_options("-Wl,--no-undefined")
        endif()
    endif()
elseif(MSVC)
    if(NOT ENABLE_RTTI)
        add_compile_options(/GR-) # Disable RTTI
    endif()
    if(ENABLE_EXCEPTIONS)
        add_compile_options(/EHsc) # Enable Exceptions
	else()
        string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # Try to remove default /EHsc cxx_flag
        add_compile_options(/D_HAS_EXCEPTIONS=0)
    endif()
endif()

# NOTE we could potentially replace this logic with COMPILE_WARNING_AS_ERROR if cmake minimum is bumped to >= 3.24
if (BUILD_WERROR)
    if (NOT MSVC)
        add_compile_options(-Werror)
    else()
        add_compile_options(/WX)
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
endif()

# Request C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(glslang_set_link_args TARGET)
    # For MinGW compiles, statically link against the GCC and C++ runtimes.
    # This avoids the need to ship those runtimes as DLLs.
    # This is supported by GCC and Clang.
    if(MINGW)
        target_link_options(${TARGET} PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endfunction()

# Root directory for build-time generated include files
set(GLSLANG_GENERATED_INCLUDEDIR "${CMAKE_BINARY_DIR}/include")

################################################################################
# Build version information generation
################################################################################
include(parse_version.cmake)
set(GLSLANG_CHANGES_FILE      "${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md")
set(GLSLANG_BUILD_INFO_H_TMPL "${CMAKE_CURRENT_SOURCE_DIR}/build_info.h.tmpl")
set(GLSLANG_BUILD_INFO_H      "${GLSLANG_GENERATED_INCLUDEDIR}/glslang/build_info.h")

parse_version(${GLSLANG_CHANGES_FILE} GLSLANG)

function(configurate_version)
    set(major ${GLSLANG_VERSION_MAJOR})
    set(minor ${GLSLANG_VERSION_MINOR})
    set(patch ${GLSLANG_VERSION_PATCH})
    set(flavor ${GLSLANG_VERSION_FLAVOR})
    configure_file(${GLSLANG_BUILD_INFO_H_TMPL} ${GLSLANG_BUILD_INFO_H} @ONLY)
endfunction()

configurate_version()

# glslang_add_build_info_dependency() adds the glslang-build-info dependency and
# generated include directories to target.
function(glslang_add_build_info_dependency target)
    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${GLSLANG_GENERATED_INCLUDEDIR}>)
endfunction()

# glslang_only_export_explicit_symbols() makes the symbol visibility hidden by
# default for <target> when building shared libraries, and sets the
# GLSLANG_IS_SHARED_LIBRARY define, and GLSLANG_EXPORTING to 1 when specifically
# building <target>.
function(glslang_only_export_explicit_symbols target)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${target} PUBLIC "GLSLANG_IS_SHARED_LIBRARY=1")
        set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET hidden)
        set_target_properties(${target} PROPERTIES C_VISIBILITY_PRESET hidden)
        if(WIN32)
            target_compile_definitions(${target} PRIVATE "GLSLANG_EXPORTING=1")
        endif()
    endif()
endfunction()

# glslang_pch() adds precompiled header rules to <target> for the pre-compiled
# header file <pch>. As target_precompile_headers() was added in CMake 3.16,
# this is a no-op if called on earlier versions of CMake.
function(glslang_pch target pch)
    if(ENABLE_PCH)
        target_precompile_headers(${target} PRIVATE ${pch})
    endif()
endfunction()

if(BUILD_EXTERNAL AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External)
    # We depend on these for later projects, so they should come first.
    add_subdirectory(External)
endif()

option(ALLOW_EXTERNAL_SPIRV_TOOLS "Allows to build against installed SPIRV-Tools-opt. This is unsupported if the commit isn't the one in known_good.json")
if(NOT TARGET SPIRV-Tools-opt)
    if(ALLOW_EXTERNAL_SPIRV_TOOLS)
        # Look for external SPIR-V Tools build, if not building in-tree
        message(STATUS "Trying to find local SPIR-V tools")
        find_package(SPIRV-Tools-opt)
        if(NOT TARGET SPIRV-Tools-opt)
            if(ENABLE_OPT)
                message(SEND_ERROR "ENABLE_OPT set but SPIR-V tools not found! Disabling SPIR-V optimization.")
            endif()
            set(ENABLE_OPT OFF)
        endif()
    else()
        if(ENABLE_OPT)
            message(SEND_ERROR "ENABLE_OPT set but SPIR-V tools not found. Please run update_glslang_sources.py, "
                "set the ALLOW_EXTERNAL_SPIRV_TOOLS option to use a local install of SPIRV-Tools, or set ENABLE_OPT=0.")
        endif()
    endif()
endif()

if(ENABLE_OPT)
    message(STATUS "optimizer enabled")
    add_compile_definitions(ENABLE_OPT=1)
else()
    if(ENABLE_HLSL)
        message(STATUS "spirv-tools not linked - illegal SPIRV may be generated for HLSL")
    endif()
    add_compile_definitions(ENABLE_OPT=0)
endif()

if(ENABLE_SPIRV)
    add_subdirectory(SPIRV)
endif()
add_subdirectory(glslang)
if(ENABLE_GLSLANG_BINARIES)
    add_subdirectory(StandAlone)
endif()

if(GLSLANG_TESTS)
    enable_testing()
    add_subdirectory(gtests)

    # glslang-testsuite runs a bash script on Windows.
    # Make sure to use '-o igncr' flag to ignore carriage returns (\r).
    set(IGNORE_CR_FLAG "")
    if(WIN32)
        set(IGNORE_CR_FLAG -o igncr)
    endif()

    if (isMultiConfig)
        set(RESULTS_PATH ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/localResults)
        set(VALIDATOR_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/$<CONFIG>/glslang)
        set(REMAP_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/$<CONFIG>/spirv-remap)
    else()
        set(RESULTS_PATH ${CMAKE_CURRENT_BINARY_DIR}/localResults)
        set(VALIDATOR_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/glslang)
        set(REMAP_PATH ${CMAKE_CURRENT_BINARY_DIR}/StandAlone/spirv-remap)
    endif()

    # The TARGET_RUNTIME_DLL_DIRS feature requires CMake 3.27 or greater.
    if(WIN32 AND BUILD_SHARED_LIBS AND CMAKE_VERSION VERSION_LESS "3.27")
        message(WARNING "The Windows shared library test configuration requires CMake 3.27 or greater")
    else()
        add_test(NAME glslang-testsuite
            COMMAND bash ${IGNORE_CR_FLAG} runtests ${RESULTS_PATH} ${VALIDATOR_PATH} ${REMAP_PATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Test/)

        # Prepend paths to shared libraries.
        if (BUILD_SHARED_LIBS)
            set_tests_properties(glslang-testsuite PROPERTIES ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:glslang-standalone>,\;>")
            set_tests_properties(glslang-testsuite PROPERTIES ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<JOIN:$<TARGET_RUNTIME_DLL_DIRS:spirv-remap>,\;>")
        endif()
    endif()
endif()

if (GLSLANG_ENABLE_INSTALL)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/glslang-config.cmake.in" [=[
        @PACKAGE_INIT@
        include(CMakeFindDependencyMacro)
        if(@ENABLE_OPT@)
            find_dependency(SPIRV-Tools-opt)
        endif()
        @INSTALL_CONFIG_UNIX@
        include("@PACKAGE_PATH_EXPORT_TARGETS@")
    ]=])

    set(PATH_EXPORT_TARGETS "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/glslang-targets.cmake")
    if(UNIX OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Fuchsia")
        set(INSTALL_CONFIG_UNIX [=[
            set(THREADS_PREFER_PTHREAD_FLAG ON)
            find_dependency(Threads)
        ]=])
    endif()
    configure_package_config_file(
        "${CMAKE_CURRENT_BINARY_DIR}/glslang-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/glslang-config.cmake"
        PATH_VARS
            PATH_EXPORT_TARGETS
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/glslang-config-version.cmake"
        VERSION ${GLSLANG_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(
        EXPORT      glslang-targets
        NAMESPACE   "glslang::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/glslang-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/glslang-config-version.cmake"
        DESTINATION
            "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )
endif()
