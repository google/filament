include_guard(GLOBAL)

# This script is run at configure time to install the latest WARP.

if(NOT WIN32)
    # WARP is only a thing on Win32
    return()
endif()


#
# At runtime, the Execution Tests look at the WARP_DLL runtime parameter to
# decide which DLL to use. When USE_NUGET_WARP is set to true, this will
# configure the tests by default to use the DLL from the installed nuget
# package.

set(USE_NUGET_WARP TRUE CACHE BOOL
    "Whether or not to use WARP from the Microsoft.Direct3D.WARP nuget package")

# The NUGET_WARP_EXTRA_ARGS cmake variable can be use to specify additional
# arguments to the nuget.exe command line.  For example, to install a specific
# version of WARP, set NUGET_WARP_EXTRA_ARGS to "-Version 1.0.13".
set(NUGET_WARP_EXTRA_ARGS "" CACHE STRING
    "Extra arguments to pass to nuget.exe when installing Microsoft.Direct3D.WARP.")

if(NOT USE_NUGET_WARP)
    message(STATUS "Using OS version of WARP")
    return()
endif()

function(InstallWarpFailure)
    message(FATAL_ERROR "Unable to install WARP nuget package. Set USE_NUGET_WARP to FALSE to use OS version of WARP.")
endfunction()

find_program(NUGET_EXE nuget.exe HINTS ${CMAKE_BINARY_DIR}/nuget)
if(NOT NUGET_EXE)
    message(STATUS "nuget.exe not found in, download nuget.exe to ${CMAKE_BINARY_DIR}/nuget/nuget.exe...")
    file(DOWNLOAD
        https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
        ${CMAKE_BINARY_DIR}/nuget/nuget.exe
    )
    find_program(NUGET_EXE nuget.exe HINTS ${CMAKE_BINARY_DIR}/nuget)
    if(NOT NUGET_EXE)
        message(SEND_ERROR "nuget.exe not found in ${CMAKE_BINARY_DIR}/nuget/nuget.exe")
        InstallWarpFailure()
    endif()
endif()

# Install the WARP nuget package.
separate_arguments(NUGET_WARP_EXTRA_ARGS)
message("Running ${NUGET_EXE} install -ForceEnglishOutput Microsoft.Direct3D.WARP -OutputDirectory ${CMAKE_BINARY_DIR}/nuget ${NUGET_WARP_EXTRA_ARGS}")

execute_process(
    COMMAND ${NUGET_EXE} install -ForceEnglishOutput Microsoft.Direct3D.WARP -OutputDirectory ${CMAKE_BINARY_DIR}/nuget ${NUGET_WARP_EXTRA_ARGS}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE nuget_output
    ERROR_VARIABLE nuget_output)

if(NOT result EQUAL 0)
    message(SEND_ERROR "nuget install Microsoft.Direct3D.WARP failed with exit code ${result}.\n${nuget_output}")
    InstallWarpFailure()
endif()

string(REGEX MATCH "Package \"(Microsoft.Direct3D.WARP..+)\" is already installed" IGNORED_OUTPUT ${nuget_output})
if(CMAKE_MATCH_1)
    set(WARP_PACKAGE ${CMAKE_MATCH_1})
else()
    string(REGEX MATCH "Added package '(Microsoft.Direct3D.WARP..+)' to folder" IGNORED_OUTPUT ${nuget_output})
    if(CMAKE_MATCH_1)
        set(WARP_PACKAGE ${CMAKE_MATCH_1})
    else()
        message(SEND_ERROR "Failed to find package install named in nuget output:\n ${nuget_output}")
        InstallWarpFailure()
    endif()
endif()

set(WARP_DIR ${CMAKE_BINARY_DIR}/nuget/${WARP_PACKAGE})

if((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ARM64") OR ("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "ARM64"))
    set(WARP_ARCH "arm64")
elseif ((CMAKE_GENERATOR_PLATFORM STREQUAL "Win32") OR ("${CMAKE_C_COMPILER_ARCHITECTURE_ID}" STREQUAL "X86"))
    set(WARP_ARCH "win32")
else()
    set(WARP_ARCH "x64")
endif()


# WARP_DLL is picked up by lit.site.cfg.in so it can be passed as a TAEF runtime
# parameter by lit.cfg
set(WARP_DLL ${WARP_DIR}/build/native/bin/${WARP_ARCH}/d3d10warp.dll)

