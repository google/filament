if(DRACO_CMAKE_COMPILER_FLAGS_CMAKE_)
  return()
endif()
set(DRACO_CMAKE_COMPILER_FLAGS_CMAKE_ 1)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include("${draco_root}/cmake/compiler_tests.cmake")

# Strings used to cache failed C/CXX flags.
set(DRACO_FAILED_C_FLAGS)
set(DRACO_FAILED_CXX_FLAGS)

# Checks C compiler for support of $c_flag. Adds $c_flag to $CMAKE_C_FLAGS when
# the compile test passes. Caches $c_flag in $DRACO_FAILED_C_FLAGS when the test
# fails.
macro(add_c_flag_if_supported c_flag)
  unset(C_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_C_FLAGS}" "${c_flag}" C_FLAG_FOUND)
  unset(C_FLAG_FAILED CACHE)
  string(FIND "${DRACO_FAILED_C_FLAGS}" "${c_flag}" C_FLAG_FAILED)

  if(${C_FLAG_FOUND} EQUAL -1 AND ${C_FLAG_FAILED} EQUAL -1)
    unset(C_FLAG_SUPPORTED CACHE)
    message("Checking C compiler flag support for: " ${c_flag})
    check_c_compiler_flag("${c_flag}" C_FLAG_SUPPORTED)
    if(${C_FLAG_SUPPORTED})
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${c_flag}" CACHE STRING "")
    else()
      set(DRACO_FAILED_C_FLAGS
          "${DRACO_FAILED_C_FLAGS} ${c_flag}"
          CACHE STRING "" FORCE)
    endif()
  endif()
endmacro()

# Checks C++ compiler for support of $cxx_flag. Adds $cxx_flag to
# $CMAKE_CXX_FLAGS when the compile test passes. Caches $c_flag in
# $DRACO_FAILED_CXX_FLAGS when the test fails.
macro(add_cxx_flag_if_supported cxx_flag)
  unset(CXX_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_CXX_FLAGS}" "${cxx_flag}" CXX_FLAG_FOUND)
  unset(CXX_FLAG_FAILED CACHE)
  string(FIND "${DRACO_FAILED_CXX_FLAGS}" "${cxx_flag}" CXX_FLAG_FAILED)

  if(${CXX_FLAG_FOUND} EQUAL -1 AND ${CXX_FLAG_FAILED} EQUAL -1)
    unset(CXX_FLAG_SUPPORTED CACHE)
    message("Checking CXX compiler flag support for: " ${cxx_flag})
    check_cxx_compiler_flag("${cxx_flag}" CXX_FLAG_SUPPORTED)
    if(${CXX_FLAG_SUPPORTED})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxx_flag}" CACHE STRING "")
    else()
      set(DRACO_FAILED_CXX_FLAGS
          "${DRACO_FAILED_CXX_FLAGS} ${cxx_flag}"
          CACHE STRING "" FORCE)
    endif()
  endif()
endmacro()

# Convenience method for adding a flag to both the C and C++ compiler command
# lines.
macro(add_compiler_flag_if_supported flag)
  add_c_flag_if_supported(${flag})
  add_cxx_flag_if_supported(${flag})
endmacro()

# Checks C compiler for support of $c_flag and terminates generation when
# support is not present.
macro(require_c_flag c_flag update_c_flags)
  unset(C_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_C_FLAGS}" "${c_flag}" C_FLAG_FOUND)

  if(${C_FLAG_FOUND} EQUAL -1)
    unset(HAVE_C_FLAG CACHE)
    message("Checking C compiler flag support for: " ${c_flag})
    check_c_compiler_flag("${c_flag}" HAVE_C_FLAG)
    if(NOT ${HAVE_C_FLAG})
      message(
        FATAL_ERROR "${PROJECT_NAME} requires support for C flag: ${c_flag}.")
    endif()
    if(${update_c_flags})
      set(CMAKE_C_FLAGS "${c_flag} ${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
    endif()
  endif()
endmacro()

# Checks CXX compiler for support of $cxx_flag and terminates generation when
# support is not present.
macro(require_cxx_flag cxx_flag update_cxx_flags)
  unset(CXX_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_CXX_FLAGS}" "${cxx_flag}" CXX_FLAG_FOUND)

  if(${CXX_FLAG_FOUND} EQUAL -1)
    unset(HAVE_CXX_FLAG CACHE)
    message("Checking CXX compiler flag support for: " ${cxx_flag})
    check_cxx_compiler_flag("${cxx_flag}" HAVE_CXX_FLAG)
    if(NOT ${HAVE_CXX_FLAG})
      message(
        FATAL_ERROR
          "${PROJECT_NAME} requires support for CXX flag: ${cxx_flag}.")
    endif()
    if(${update_cxx_flags})
      set(CMAKE_CXX_FLAGS
          "${cxx_flag} ${CMAKE_CXX_FLAGS}"
          CACHE STRING "" FORCE)
    endif()
  endif()
endmacro()

# Checks for support of $flag by both the C and CXX compilers. Terminates
# generation when support is not present in both compilers.
macro(require_compiler_flag flag update_cmake_flags)
  require_c_flag(${flag} ${update_cmake_flags})
  require_cxx_flag(${flag} ${update_cmake_flags})
endmacro()

# Checks only non-MSVC targets for support of $c_flag and terminates generation
# when support is not present.
macro(require_c_flag_nomsvc c_flag update_c_flags)
  if(NOT MSVC)
    require_c_flag(${c_flag} ${update_c_flags})
  endif()
endmacro()

# Checks only non-MSVC targets for support of $cxx_flag and terminates
# generation when support is not present.
macro(require_cxx_flag_nomsvc cxx_flag update_cxx_flags)
  if(NOT MSVC)
    require_cxx_flag(${cxx_flag} ${update_cxx_flags})
  endif()
endmacro()

# Checks only non-MSVC targets for support of $flag by both the C and CXX
# compilers. Terminates generation when support is not present in both
# compilers.
macro(require_compiler_flag_nomsvc flag update_cmake_flags)
  require_c_flag_nomsvc(${flag} ${update_cmake_flags})
  require_cxx_flag_nomsvc(${flag} ${update_cmake_flags})
endmacro()

# Adds $flag to assembler command line.
macro(append_as_flag flag)
  unset(AS_FLAG_FOUND CACHE)
  string(FIND "${DRACO_AS_FLAGS}" "${flag}" AS_FLAG_FOUND)

  if(${AS_FLAG_FOUND} EQUAL -1)
    set(DRACO_AS_FLAGS "${DRACO_AS_FLAGS} ${flag}")
  endif()
endmacro()

# Adds $flag to the C compiler command line.
macro(append_c_flag flag)
  unset(C_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_C_FLAGS}" "${flag}" C_FLAG_FOUND)

  if(${C_FLAG_FOUND} EQUAL -1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
  endif()
endmacro()

# Adds $flag to the CXX compiler command line.
macro(append_cxx_flag flag)
  unset(CXX_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" CXX_FLAG_FOUND)

  if(${CXX_FLAG_FOUND} EQUAL -1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  endif()
endmacro()

# Adds $flag to the C and CXX compiler command lines.
macro(append_compiler_flag flag)
  append_c_flag(${flag})
  append_cxx_flag(${flag})
endmacro()

# Adds $flag to the executable linker command line.
macro(append_exe_linker_flag flag)
  unset(LINKER_FLAG_FOUND CACHE)
  string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "${flag}" LINKER_FLAG_FOUND)

  if(${LINKER_FLAG_FOUND} EQUAL -1)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
  endif()
endmacro()

# Adds $flag to the link flags for $target.
function(append_link_flag_to_target target flags)
  unset(target_link_flags)
  get_target_property(target_link_flags ${target} LINK_FLAGS)

  if(target_link_flags)
    unset(link_flag_found)
    string(FIND "${target_link_flags}" "${flags}" link_flag_found)

    if(NOT ${link_flag_found} EQUAL -1)
      return()
    endif()

    set(target_link_flags "${target_link_flags} ${flags}")
  else()
    set(target_link_flags "${flags}")
  endif()

  set_target_properties(${target} PROPERTIES LINK_FLAGS ${target_link_flags})
endfunction()

# Adds $flag to executable linker flags, and makes sure C/CXX builds still work.
macro(require_linker_flag flag)
  append_exe_linker_flag(${flag})

  unset(c_passed)
  draco_check_c_compiles("LINKER_FLAG_C_TEST(${flag})" "" c_passed)
  unset(cxx_passed)
  draco_check_cxx_compiles("LINKER_FLAG_CXX_TEST(${flag})" "" cxx_passed)

  if(NOT c_passed OR NOT cxx_passed)
    message(FATAL_ERROR "Linker flag test for ${flag} failed.")
  endif()
endmacro()
