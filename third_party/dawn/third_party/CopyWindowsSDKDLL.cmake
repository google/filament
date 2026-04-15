# Copyright 2025 The Dawn & Tint Authors
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

# Function to detect Windows SDK path and version
# Returns WIN10_SDK_PATH and WIN10_SDK_VERSION via output parameters
function(DetectWindowsSDK out_sdk_path out_sdk_version)
    message(STATUS "Finding Windows SDK Directory")

    message(STATUS "Display environment variables:")
    execute_process(COMMAND ${CMAKE_COMMAND} -E environment COMMAND_ECHO STDOUT)

    if(DEFINED ENV{WINDOWSSDKDIR})
        # If WINDOWSSDKDIR env var is defined, use its value. This is defined, for example, when
        # using a Visual Studio command prompt.
        set(sdk_path "$ENV{WINDOWSSDKDIR}")
        message(STATUS "Found WINDOWSSDKDIR environment variable: $ENV{WINDOWSSDKDIR}")
    else()
        # There's no easy way to get the Windows SDK path in CMake; however, conveniently, DXC
        # contains a FindD3D12.cmake file that returns WIN10_SDK_PATH and WIN10_SDK_VERSION,
        # so let's use that.
        # TODO(crbug.com/tint/2106): Get the Win10 SDK path and version ourselves until
        # dxc/cmake/modules/FindD3D12.cmake supports non-VS generators.
        get_filename_component(sdk_path "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
        message(STATUS "WINDOWSSDKDIR environment variable is not defined, retrieving from registry: ${sdk_path}")
    endif()

    message(STATUS "Finding Windows SDK version")
    if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(sdk_version ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
        message(STATUS "Found Windows SDK version from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION': ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
    else()
        # CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION may not be defined if, for example,
        # the Ninja generator is used instead of Visual Studio. Attempt to retrieve the
        # most recent SDK version from the list of paths under "${WIN10_SDK_PATH}/Include/".
        file(GLOB sdk_dirs RELATIVE "${sdk_path}/Include/" "${sdk_path}/Include/10.*")
        if (sdk_dirs)
            list(POP_BACK sdk_dirs sdk_version)
        endif()
        unset(sdk_dirs)
        message(STATUS "Windows SDK version not found from CMake variable 'CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION', attempted to find from SDK path: ${sdk_version}")
    endif()

    # Return values via output parameters
    set(${out_sdk_path} ${sdk_path} PARENT_SCOPE)
    set(${out_sdk_version} ${sdk_version} PARENT_SCOPE)
endfunction()

# Function to add a target that copies a DLL from Windows SDK to main build directory
# Parameters:
#   - target_name: Name of the custom target to create
#   - dll_name: Name of the DLL (e.g., "dxil.dll", "d3dcompiler_47.dll")
function(AddCopyWindowsSDKDLLTarget target_name dll_name)
    if (NOT WIN32)
        message(FATAL_ERROR "AddCopyWindowsSDKDLLTarget can only be called on Windows")
    endif()

    DetectWindowsSDK(WIN10_SDK_PATH WIN10_SDK_VERSION)

    set(DLL_PATH "${WIN10_SDK_PATH}/bin/${WIN10_SDK_VERSION}/x64/${dll_name}")

    # Copy directly to main build directory
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (isMultiConfig)
        set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>")
    else()
        set(OUTPUT_DIR "${CMAKE_BINARY_DIR}")
    endif()
    set(DEST_PATH "${OUTPUT_DIR}/${dll_name}")

    add_custom_target(${target_name}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_PATH} ${DEST_PATH}
        COMMENT "Copying ${DLL_PATH} to ${DEST_PATH}")
endfunction()
