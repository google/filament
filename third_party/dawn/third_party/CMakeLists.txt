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

# Don't build testing in third_party dependencies
set(BUILD_TESTING OFF)

# fetch_dawn_dependencies.py is an alternative to using depot_tools
# It is particularly interesting when building dawn as a subdirectory in
# a parent project that does not want to use depot_tools.
if (DAWN_FETCH_DEPENDENCIES)
    set(EXTRA_FETCH_ARGS)
    if (NOT TARGET gmock AND (DAWN_BUILD_TESTS OR TINT_BUILD_TESTS))
        list(APPEND EXTRA_FETCH_ARGS --use-test-deps)
    endif()

    message(STATUS "Running fetch_dawn_dependencies:")
    execute_process(
        COMMAND
            ${Python3_EXECUTABLE}
            "${PROJECT_SOURCE_DIR}/tools/fetch_dawn_dependencies.py"
            --directory ${PROJECT_SOURCE_DIR}
            ${EXTRA_FETCH_ARGS}
    )
endif ()

################################################################################
# Start of Emscripten enabled third party directories
#   To minimize the number of third party targets to make when building with
#   Emscripten to the minimal set, only subdirectories added within the Start
#   and End of the Emscripten block are enabled .
################################################################################

if (NOT TARGET absl::strings)
    # Recommended setting for compability with future abseil releases.
    set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
    message(STATUS "Dawn: using Abseil at ${DAWN_ABSEIL_DIR}")
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
        ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
        add_compile_options(
            -Wno-array-parameter
            -Wno-deprecated-builtins
            -Wno-unknown-warning-option
        )
    endif()

    add_subdirectory(${DAWN_ABSEIL_DIR} "${CMAKE_CURRENT_BINARY_DIR}/abseil")
endif()

if ((DAWN_BUILD_TESTS OR TINT_BUILD_TESTS) AND NOT TARGET gmock)
    set(gtest_force_shared_crt ON CACHE BOOL "Controls whether a shared run-time library should be used even when Google Test is built as static library" FORCE)
    if (EMSCRIPTEN)
        # For Emscripten builds, we need to disable pthreads.
        set(gtest_disable_pthreads ON)
    endif()

    # We don't use exceptions, so we don't need gtest to handle them. Emscripten
    # builds in particular, with JSPI enabled, crash without this enabled.
    # There doesn't seem to be an option to tell the gtest build files to do
    # this, so we set it globally. (This seems to be the intended usage.)
    add_definitions(-DGTEST_HAS_EXCEPTIONS=0)

    add_subdirectory(${DAWN_GOOGLETEST_DIR} "${CMAKE_CURRENT_BINARY_DIR}/googletest" EXCLUDE_FROM_ALL)
endif()

################################################################################
# End of Emscripten enabled third party directories
################################################################################
if (EMSCRIPTEN)
    return()
endif()

if (NOT TARGET SPIRV-Headers)
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using SPIRV-Headers at ${DAWN_SPIRV_HEADERS_DIR}")
    add_subdirectory(${DAWN_SPIRV_HEADERS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/spirv-headers")
endif()

if (NOT TARGET SPIRV-Tools AND
    (((DAWN_ENABLE_OPENGL OR DAWN_ENABLE_VULKAN)
        AND DAWN_ENABLE_SPIRV_VALIDATION)
        OR TINT_BUILD_SPV_READER
        OR TINT_BUILD_SPV_WRITER
        OR DAWN_USE_BUILT_DXC))
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "Controls whether SPIR-V tests are run" FORCE)
    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
    set(SPIRV_WERROR OFF CACHE BOOL OFF FORCE)

    message(STATUS "Dawn: using SPIRV-Tools at ${DAWN_SPIRV_TOOLS_DIR}")
    add_subdirectory(${DAWN_SPIRV_TOOLS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/spirv-tools" EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET glslang AND (TINT_BUILD_GLSL_WRITER OR TINT_BUILD_GLSL_VALIDATOR) AND TINT_BUILD_CMD_TOOLS)
    set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
    set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
    message(STATUS "Dawn: using GLSLang at ${DAWN_GLSLANG_DIR}")
    # https://github.com/KhronosGroup/glslang/issues/2283
    # glslang does not export symbols properly when BUILD_SHARED_LIBS=1, so always build it as static for now.
    set(BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS 0)
    add_subdirectory("${DAWN_GLSLANG_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/glslang" EXCLUDE_FROM_ALL)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_SAVED})
endif()

if (NOT TARGET glfw AND DAWN_USE_GLFW)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 ${DAWN_USE_X11} CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND ${DAWN_USE_WAYLAND} CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using GLFW at ${DAWN_GLFW_DIR}")
    add_subdirectory(${DAWN_GLFW_DIR} "${CMAKE_CURRENT_BINARY_DIR}/glfw")
endif()

if (DAWN_BUILD_PROTOBUF AND NOT TARGET libprotobuf-mutator)
    message(STATUS "Dawn: using LPM at ${DAWN_LPM_DIR}")
    include("libprotobuf-mutator/BUILD.cmake")
endif()

if (DAWN_ENABLE_DESKTOP_GL OR DAWN_ENABLE_OPENGLES)
    # Header-only library for khrplatform.h
    add_library(dawn_khronos_platform INTERFACE)
    target_sources(dawn_khronos_platform INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/khronos/EGL-Registry/api/KHR/khrplatform.h")
    target_include_directories(dawn_khronos_platform INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/khronos/EGL-Registry/api")
endif()

if (NOT TARGET Vulkan-Headers AND DAWN_ENABLE_VULKAN)
    message(STATUS "Dawn: using Vulkan-Headers at ${DAWN_VULKAN_HEADERS_DIR}")

    set(VULKAN_HEADERS_ENABLE_MODULE OFF)
    add_subdirectory(${DAWN_VULKAN_HEADERS_DIR} "${CMAKE_CURRENT_BINARY_DIR}/vulkan-headers")
endif()

if (NOT TARGET VulkanUtilityHeaders AND DAWN_ENABLE_VULKAN)
    message(STATUS "Dawn: using VulkanUtilityLibraries at ${DAWN_VULKAN_UTILITY_LIBRARIES_DIR}")
    add_subdirectory(${DAWN_VULKAN_UTILITY_LIBRARIES_DIR} "${CMAKE_CURRENT_BINARY_DIR}/vulkan-utility-libraries")
endif()

if (DAWN_ENABLE_SWIFTSHADER AND NOT TARGET vk_swiftshader)
    set(SWIFTSHADER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SWIFTSHADER_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

    message(STATUS "Dawn: using Swiftshader at ${DAWN_SWIFTSHADER_DIR}")
    add_subdirectory(${DAWN_SWIFTSHADER_DIR} "${CMAKE_CURRENT_BINARY_DIR}/swiftshader")
endif()

if (TINT_BUILD_BENCHMARKS OR DAWN_BUILD_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING FALSE CACHE BOOL FALSE FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/google_benchmark/src EXCLUDE_FROM_ALL)
endif()

# Populates 'targets' with list of all targets under 'dir'
# Use 'get_all_targets_recursive' instead of this macro
macro(do_get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        do_get_all_targets_recursive(${targets} ${subdir})
    endforeach()
    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()
function(get_all_targets_recursive targets dir)
    set(targets_out)
    do_get_all_targets_recursive(targets_out ${dir})
    set(${targets} ${targets_out} PARENT_SCOPE)
endfunction()

function(AddSubdirectoryDXC)
    # We use a CMake function so that all these (non-cache) variables are scoped
    # only to this function.
    set(HLSL_OPTIONAL_PROJS_IN_DEFAULT OFF)
    set(HLSL_ENABLE_ANALYZE OFF)
    set(HLSL_OFFICIAL_BUILD OFF)
    set(HLSL_ENABLE_FIXED_VER OFF)
    set(HLSL_BUILD_DXILCONV OFF)
    set(HLSL_INCLUDE_TESTS OFF)
    # When bundling up dawn, the object files of dxc must be binary compatible with rest of dawn.
    # The default value of `OFF` adds a macro `_ITERATOR_DEBUG_LEVEL=0`.
    # This is a problem in `Debug` builds of dawn because all of dawn's objects get compiled
    # with `_ITERATOR_DEBUG_LEVEL=2`
    set(HLSL_ENABLE_DEBUG_ITERATORS ON)

    set(ENABLE_SPIRV_CODEGEN OFF)
    set(SPIRV_BUILD_TESTS OFF)

    set(LLVM_BUILD_RUNTIME ON)
    set(LLVM_BUILD_EXAMPLES OFF)
    set(LLVM_BUILD_TESTS OFF)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_INCLUDE_DOCS OFF)
    set(LLVM_INCLUDE_EXAMPLES OFF)
    set(LLVM_OPTIMIZED_TABLEGEN OFF)
    set(LLVM_APPEND_VC_REV OFF)
    # Enable exception handling (requires RTTI)
    set(LLVM_ENABLE_RTTI ON)
    set(LLVM_ENABLE_EH ON)
    set(CLANG_CL OFF)

    if (DAWN_ENABLE_ASAN AND DAWN_ENABLE_UBSAN)
        set(LLVM_USE_SANITIZER "Address;Undefined")
    elseif(DAWN_ENABLE_ASAN)
        set(LLVM_USE_SANITIZER "Address")
    elseif(DAWN_ENABLE_UBSAN)
        set(LLVM_USE_SANITIZER "Undefined")
    endif()

    # Cache variables -- these are *not* scoped to this function
    set(LLVM_TARGETS_TO_BUILD "None" CACHE STRING "" FORCE)
    set(LLVM_DEFAULT_TARGET_TRIPLE "dxil-ms-dx" CACHE STRING "" FORCE)
    set(CLANG_ENABLE_STATIC_ANALYZER OFF CACHE BOOL "" FORCE)
    set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "" FORCE)
    set(CLANG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "" FORCE)

    if (NOT WIN32)
        set(DIRECTX_HEADER_INCLUDE_DIR "${DAWN_THIRD_PARTY_DIR}/dxheaders/include")
    endif()

    # Disable HCT.cmake looking for and using clang-format. This is used to compare generated files
    # against the copy that is committed to the repo, but fails because the DXC .clangformat file is
    # not visible from our build dir. We don't need this validation, so just disable it.
    set(CLANG_FORMAT_EXE "" CACHE STRING "" FORCE)

    # Override RPATH so that it points to current dir (exe path). This allows both executable and
    # shared library to be in the same location, which we set below. Note that DXC places places
    # executables in a bin directory, and shared libraries in a lib directory.
    if (APPLE)
        set(CMAKE_INSTALL_NAME_DIR "@rpath")
        set(CMAKE_INSTALL_RPATH "@executable_path")
    else()
        set(CMAKE_INSTALL_RPATH "\$ORIGIN")
    endif()

    message(STATUS "\nAdding DXC to build:\n")
    add_subdirectory(dxc
        # Disable all targets by default, and enable only the dxc and dxcompiler targets (below)
        EXCLUDE_FROM_ALL
    )
    set_target_properties(dxc PROPERTIES EXCLUDE_FROM_ALL FALSE)
    set_target_properties(dxcompiler PROPERTIES EXCLUDE_FROM_ALL FALSE)

    if ((NOT IS_DEBUG_BUILD) AND DAWN_DXC_ENABLE_ASSERTS_IN_NDEBUG)
        # NOTE: Don't set LLVM_ENABLE_ASSERTIONS because it modifies CMAKE_CXX_FLAGS_* by removing NDEBUG
        # from it; but this is a global CMake flag that affects all targets, including the non-DXC ones.
        # Instead, we add '-UNDEBUG', which will come after the default '-DNDEBUG' that comes from
        # CMAKE_CXX_FLAGS_*, and rely on the fact that compilers will undefine NDEBUG if it comes last
        # on the compile command line. This was tested on Clang, GCC, and MSVC.
        # Also note that MSVC will warn about both being defined, which is why DXC/LLVM
        # removes NDEBUG from CMAKE_CXX_FLAGS_*, but we can live with the warnings.
        # TODO(crbug.com/358169277): Once we move the 'dxc' directory one level deeper,
        # use add_definitions instead of getting all targets and setting properties on them.
        get_all_targets_recursive(ALL_TARGETS ${CMAKE_CURRENT_SOURCE_DIR}/dxc)
        foreach(t IN LISTS ALL_TARGETS)
            set_property(TARGET ${t} APPEND PROPERTY COMPILE_OPTIONS "-UNDEBUG")
            if (NOT MSVC)
                set_property(TARGET ${t} APPEND PROPERTY COMPILE_DEFINITIONS "_DEBUG")
            endif()
            # Reduce binary size by dropping assertion strings
            set_property(TARGET ${t} APPEND PROPERTY COMPILE_DEFINITIONS "LLVM_ASSERTIONS_NO_STRINGS")
            # Always trap on asserts for consistent crash logging
            set_property(TARGET ${t} APPEND PROPERTY COMPILE_DEFINITIONS "LLVM_ASSERTIONS_TRAP")
        endforeach()
    endif()

    # Override output dir for both targets so that they end up in the same place
    # as the rest of our binaries. Otherwise, DXC wants its outputs in bin and lib dirs.
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (isMultiConfig)
        set_target_properties(dxc dxcompiler PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_DEBUG" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "RUNTIME_OUTPUT_DIRECTORY_RELEASE" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "LIBRARY_OUTPUT_DIRECTORY_DEBUG" "${CMAKE_BINARY_DIR}/$<CONFIG>"
            "LIBRARY_OUTPUT_DIRECTORY_RELEASE" "${CMAKE_BINARY_DIR}/$<CONFIG>"
        )
    else()
        set_target_properties(dxc dxcompiler PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY" "${CMAKE_BINARY_DIR}"
            "LIBRARY_OUTPUT_DIRECTORY" "${CMAKE_BINARY_DIR}"
        )
    endif()

    # Create a target that copies dxil.dll from the platform SDK
    if (WIN32)
        message(STATUS "Finding Windows SDK Directory")

        message(STATUS "Display environment variables:")
        execute_process(COMMAND ${CMAKE_COMMAND} -E environment COMMAND_ECHO STDOUT)

        if(DEFINED ENV{WINDOWSSDKDIR})
            # If WINDOWSSDKDIR env var is defined, use its value. This is defined, for example, when
            # using a Visual Studio command prompt.
            set(WIN10_SDK_PATH "$ENV{WINDOWSSDKDIR}")
            message(STATUS "Found WINDOWSSDKDIR environment variable: ${WIN10_SDK_PATH}")
        else()
            # There's no easy way to get the Windows SDK path in CMake; however, conveniently, DXC
            # contains a FindD3D12.cmake file that returns WIN10_SDK_PATH and WIN10_SDK_VERSION,
            # so let's use that.
            # TODO(crbug.com/tint/2106): Get the Win10 SDK path and version ourselves until
            # dxc/cmake/modules/FindD3D12.cmake supports non-VS generators.
            get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
            message(STATUS "WINDOWSSDKDIR environment variable is not defined, retrieving from registry: ${WIN10_SDK_PATH}")
        endif()

        message(STATUS "Finding Windows SDK version")
        if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
            set (WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
            message(STATUS "Found Windows SDK version from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION': ${WIN10_SDK_VERSION}")
        else()
            # CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION may not be defined if, for example,
            # the Ninja generator is used instead of Visual Studio. Attempt to retrieve the
            # most recent SDK version from the list of paths under "${WIN10_SDK_PATH}/Include/".
            file(GLOB sdk_dirs RELATIVE "${WIN10_SDK_PATH}/Include/" "${WIN10_SDK_PATH}/Include/10.*")
            if (sdk_dirs)
                list(POP_BACK sdk_dirs WIN10_SDK_VERSION)
            endif()
            unset(sdk_dirs)
            message(STATUS "Windows SDK version not found from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION', attempted to find from SDK path: ${WIN10_SDK_VERSION}")
        endif()

        set(DXIL_DLL_PATH "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x64/dxil.dll")
        add_custom_target(copy_dxil_dll)
        add_custom_command(
            TARGET copy_dxil_dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DXIL_DLL_PATH} $<TARGET_FILE_DIR:dxcompiler>
            COMMENT "Copying ${DXIL_DLL_PATH} to $<TARGET_FILE_DIR:dxcompiler>")
        # Make dxc target depend on copy_dxil_dll
        add_dependencies(dxc copy_dxil_dll)
    endif()
endfunction()

if (DAWN_USE_BUILT_DXC)
    AddSubdirectoryDXC()
endif()

if (TINT_BUILD_TINTD)
    set(LANGSVR_JSON_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp" EXCLUDE_FROM_ALL)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/langsvr" EXCLUDE_FROM_ALL)
endif()
