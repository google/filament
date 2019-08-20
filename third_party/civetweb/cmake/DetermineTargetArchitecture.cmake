# - Determines the target architecture of the compilation
#
# This function checks the architecture that will be built by the compiler
# and sets a variable to the architecture
#
#  determine_target_architecture(<OUTPUT_VAR>)
#
# - Example
#
# include(DetermineTargetArchitecture)
# determine_target_architecture(PROJECT_NAME_ARCHITECTURE)

if(__determine_target_architecture)
  return()
endif()
set(__determine_target_architecture INCLUDED)

function(determine_target_architecture FLAG)
  if (MSVC)
    if("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "X86")
      set(ARCH "i686")
    elseif("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "x64")
      set(ARCH "x86_64")
    elseif("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "ARM")
      set(ARCH "arm")
    else()
      message(FATAL_ERROR "Failed to determine the MSVC target architecture: ${MSVC_C_ARCHITECTURE_ID}")
    endif()
  else()
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -dumpmachine
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE ARCH
      ERROR_QUIET
    )
    if (RESULT)
      message(FATAL_ERROR "Failed to determine target architecture triplet: ${RESULT}")
    endif()
    string(REGEX MATCH "([^-]+).*" ARCH_MATCH ${ARCH})
    if (NOT CMAKE_MATCH_1 OR NOT ARCH_MATCH)
      message(FATAL_ERROR "Failed to match the target architecture triplet: ${ARCH}")
    endif()
    set(ARCH ${CMAKE_MATCH_1})
  endif()
  message(STATUS "Target architecture - ${ARCH}")
  set(FLAG ${ARCH} PARENT_SCOPE)
endfunction()
