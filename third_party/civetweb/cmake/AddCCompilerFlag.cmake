# - Adds a compiler flag if it is supported by the compiler
#
# This function checks that the supplied compiler flag is supported and then
# adds it to the corresponding compiler flags
#
#  add_c_compiler_flag(<FLAG> [<VARIANT>])
#
# - Example
#
# include(AddCCompilerFlag)
# add_c_compiler_flag(-Wall)
# add_c_compiler_flag(-no-strict-aliasing RELEASE)
# Requires CMake 2.6+

if(__add_c_compiler_flag)
  return()
endif()
set(__add_c_compiler_flag INCLUDED)

include(CheckCCompilerFlag)

function(add_c_compiler_flag FLAG)
  string(TOUPPER "HAVE_C_FLAG_${FLAG}" SANITIZED_FLAG)
  string(REPLACE "+" "X" SANITIZED_FLAG ${SANITIZED_FLAG})
  string(REGEX REPLACE "[^A-Za-z_0-9]" "_" SANITIZED_FLAG ${SANITIZED_FLAG})
  string(REGEX REPLACE "_+" "_" SANITIZED_FLAG ${SANITIZED_FLAG})
  set(CMAKE_REQUIRED_FLAGS "${FLAG}")
  check_c_compiler_flag("" ${SANITIZED_FLAG})
  if(${SANITIZED_FLAG})
    set(VARIANT ${ARGV1})
    if(ARGV1)
      string(REGEX REPLACE "[^A-Za-z_0-9]" "_" VARIANT "${VARIANT}")
      string(TOUPPER "_${VARIANT}" VARIANT)
    endif()
    set(CMAKE_C_FLAGS${VARIANT} "${CMAKE_C_FLAGS${VARIANT}} ${FLAG}" PARENT_SCOPE)
  endif()
endfunction()

