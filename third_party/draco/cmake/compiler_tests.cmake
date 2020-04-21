if(DRACO_CMAKE_COMPILER_TESTS_CMAKE_)
  return()
endif()
set(DRACO_CMAKE_COMPILER_TESTS_CMAKE_ 1)

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

# The basic main() macro used in all compile tests.
set(DRACO_C_MAIN "\nint main(void) { return 0; }")
set(DRACO_CXX_MAIN "\nint main() { return 0; }")

# Strings containing the names of passed and failed tests.
set(DRACO_C_PASSED_TESTS)
set(DRACO_C_FAILED_TESTS)
set(DRACO_CXX_PASSED_TESTS)
set(DRACO_CXX_FAILED_TESTS)

macro(draco_push_var var new_value)
  set(SAVED_${var} ${var})
  set(${var} ${new_value})
endmacro()

macro(draco_pop_var var)
  set(var ${SAVED_${var}})
  unset(SAVED_${var})
endmacro()

# Confirms $test_source compiles and stores $test_name in one of
# $DRACO_C_PASSED_TESTS or $DRACO_C_FAILED_TESTS depending on out come. When the
# test passes $result_var is set to 1. When it fails $result_var is unset. The
# test is not run if the test name is found in either of the passed or failed
# test variables.
macro(draco_check_c_compiles test_name test_source result_var)
  unset(C_TEST_PASSED CACHE)
  unset(C_TEST_FAILED CACHE)
  string(FIND "${DRACO_C_PASSED_TESTS}" "${test_name}" C_TEST_PASSED)
  string(FIND "${DRACO_C_FAILED_TESTS}" "${test_name}" C_TEST_FAILED)
  if(${C_TEST_PASSED} EQUAL -1 AND ${C_TEST_FAILED} EQUAL -1)
    unset(C_TEST_COMPILED CACHE)
    message("Running C compiler test: ${test_name}")
    check_c_source_compiles("${test_source} ${DRACO_C_MAIN}" C_TEST_COMPILED)
    set(${result_var} ${C_TEST_COMPILED})

    if(${C_TEST_COMPILED})
      set(DRACO_C_PASSED_TESTS "${DRACO_C_PASSED_TESTS} ${test_name}")
    else()
      set(DRACO_C_FAILED_TESTS "${DRACO_C_FAILED_TESTS} ${test_name}")
      message("C Compiler test ${test_name} failed.")
    endif()
  elseif(NOT ${C_TEST_PASSED} EQUAL -1)
    set(${result_var} 1)
  else() # ${C_TEST_FAILED} NOT EQUAL -1
    unset(${result_var})
  endif()
endmacro()

# Confirms $test_source compiles and stores $test_name in one of
# $DRACO_CXX_PASSED_TESTS or $DRACO_CXX_FAILED_TESTS depending on out come. When
# the test passes $result_var is set to 1. When it fails $result_var is unset.
# The test is not run if the test name is found in either of the passed or
# failed test variables.
macro(draco_check_cxx_compiles test_name test_source result_var)
  unset(CXX_TEST_PASSED CACHE)
  unset(CXX_TEST_FAILED CACHE)
  string(FIND "${DRACO_CXX_PASSED_TESTS}" "${test_name}" CXX_TEST_PASSED)
  string(FIND "${DRACO_CXX_FAILED_TESTS}" "${test_name}" CXX_TEST_FAILED)
  if(${CXX_TEST_PASSED} EQUAL -1 AND ${CXX_TEST_FAILED} EQUAL -1)
    unset(CXX_TEST_COMPILED CACHE)
    message("Running CXX compiler test: ${test_name}")
    check_cxx_source_compiles("${test_source} ${DRACO_CXX_MAIN}"
                              CXX_TEST_COMPILED)
    set(${result_var} ${CXX_TEST_COMPILED})

    if(${CXX_TEST_COMPILED})
      set(DRACO_CXX_PASSED_TESTS "${DRACO_CXX_PASSED_TESTS} ${test_name}")
    else()
      set(DRACO_CXX_FAILED_TESTS "${DRACO_CXX_FAILED_TESTS} ${test_name}")
      message("CXX Compiler test ${test_name} failed.")
    endif()
  elseif(NOT ${CXX_TEST_PASSED} EQUAL -1)
    set(${result_var} 1)
  else() # ${CXX_TEST_FAILED} NOT EQUAL -1
    unset(${result_var})
  endif()
endmacro()

# Convenience macro that confirms $test_source compiles as C and C++.
# $result_var is set to 1 when both tests are successful, and 0 when one or both
# tests fail. Note: This macro is intended to be used to write to result
# variables that are expanded via configure_file(). $result_var is set to 1 or 0
# to allow direct usage of the value in generated source files.
macro(draco_check_source_compiles test_name test_source result_var)
  unset(C_PASSED)
  unset(CXX_PASSED)
  draco_check_c_compiles(${test_name} ${test_source} C_PASSED)
  draco_check_cxx_compiles(${test_name} ${test_source} CXX_PASSED)
  if(${C_PASSED} AND ${CXX_PASSED})
    set(${result_var} 1)
  else()
    set(${result_var} 0)
  endif()
endmacro()
