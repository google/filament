# if coverage reports are not enabled, skip all of this
if(NOT LLVM_BUILD_INSTRUMENTED_COVERAGE)
  return()
endif()

file(TO_NATIVE_PATH
     "${LLVM_SOURCE_DIR}/utils/prepare-code-coverage-artifact.py"
     PREPARE_CODE_COV_ARTIFACT)

# llvm-cov and llvm-profdata need to match the host compiler. They can either be
# explicitly provided by the user, or we will look them up based on the install
# location of the C++ compiler.

# HLSL Change Begin - This is probably worth upstreaming. Some Linux packages
# install versions of the LLVM tools that are versioned. This handles that case.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  string(REGEX REPLACE "(^[0-9]+)(\\.[0-9\\.]+)" "-\\1" HOST_LLVM_VERSION_SUFFIX ${CMAKE_CXX_COMPILER_VERSION})
endif()

get_filename_component(COMPILER_DIRECTORY ${CMAKE_CXX_COMPILER} DIRECTORY)
find_program(LLVM_COV NAMES llvm-cov llvm-cov${HOST_LLVM_VERSION_SUFFIX} PATHS ${COMPILER_DIRECTORY} NO_DEFAULT_PATH)
find_program(LLVM_PROFDATA NAMES llvm-profdata llvm-profdata${HOST_LLVM_VERSION_SUFFIX} PATHS ${COMPILER_DIRECTORY} NO_DEFAULT_PATH)
# HLSL Change End - Detect versioned tools.

if(NOT LLVM_COV OR NOT LLVM_PROFDATA)
  message(WARNING "Could not find code coverage tools, skipping generating targets. You may explicitly specify LLVM_COV and LLVM_PROFDATA to work around this warning.")
  return()
endif()

set(LLVM_CODE_COVERAGE_TARGETS "" CACHE STRING "Targets to generate coverage reports against (defaults to all exported targets if empty)")
mark_as_advanced(LLVM_CODE_COVERAGE_TARGETS)

# HLSL Change Begin - This is probably worth upstreaming...
set(LLVM_CODE_COVERAGE_TEST_TARGETS "" CACHE STRING "Targets to run to generate coverage profiles.")
mark_as_advanced(LLVM_CODE_COVERAGE_TEST_TARGETS)

if (LLVM_CODE_COVERAGE_TEST_TARGETS)
  set(COV_DEPENDS DEPENDS ${LLVM_CODE_COVERAGE_TEST_TARGETS})
endif()
# HLSL Change End

if(NOT LLVM_CODE_COVERAGE_TARGETS)
  # by default run the coverage report across all the exports provided
  get_property(COV_TARGETS GLOBAL PROPERTY LLVM_EXPORTS)
endif()

file(TO_NATIVE_PATH
     "${CMAKE_BINARY_DIR}/report/"
     REPORT_DIR)

foreach(target ${LLVM_CODE_COVERAGE_TARGETS} ${COV_TARGETS})
  get_target_property(target_type ${target} TYPE)
  if("${target_type}" STREQUAL "SHARED_LIBRARY" OR "${target_type}" STREQUAL "EXECUTABLE")
    list(APPEND coverage_binaries $<TARGET_FILE:${target}>)
  endif()
endforeach()

set(LLVM_COVERAGE_SOURCE_DIRS "" CACHE STRING "Source directories to restrict coverage reports to.")
mark_as_advanced(LLVM_COVERAGE_SOURCE_DIRS)

foreach(dir ${LLVM_COVERAGE_SOURCE_DIRS})
  list(APPEND restrict_flags -restrict ${dir})
endforeach()

# Utility target to clear out profile data.
# This isn't connected to any dependencies because it is a bit finicky to get
# working exactly how a user might want.
add_custom_target(clear-profile-data
                  COMMAND ${CMAKE_COMMAND} -E
                          remove_directory ${LLVM_PROFILE_DATA_DIR})

# This currently only works for LLVM, but could be expanded to work for all
# sub-projects. The current limitation is based on not having a good way to
# automaticall plumb through the targets that we want to run coverage against.
add_custom_target(generate-coverage-report
                  COMMAND ${Python3_EXECUTABLE} ${PREPARE_CODE_COV_ARTIFACT}
                          ${LLVM_PROFDATA} ${LLVM_COV} ${LLVM_PROFILE_DATA_DIR}
                          ${REPORT_DIR} ${coverage_binaries}
                          --unified-report ${restrict_flags}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  ${COV_DEPENDS}) # Run tests
