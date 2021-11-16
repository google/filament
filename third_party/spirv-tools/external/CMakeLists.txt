# Copyright (c) 2015-2016 The Khronos Group Inc.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Utility functions for pushing & popping variables.
function(push_variable var val)
    set("${var}_SAVE_STACK" "${${var}}" "${${var}_SAVE_STACK}" PARENT_SCOPE)
    set(${var} ${val} PARENT_SCOPE)
endfunction()
function(pop_variable var)
    set(save_stack "${${var}_SAVE_STACK}")
    list(GET save_stack 0 val)
    list(REMOVE_AT save_stack 0)
    set("${var}_SAVE_STACK" "${save_stack}" PARENT_SCOPE)
    set(${var} ${val} PARENT_SCOPE)
endfunction()

if (DEFINED SPIRV-Headers_SOURCE_DIR)
  # This allows flexible position of the SPIRV-Headers repo.
  set(SPIRV_HEADER_DIR ${SPIRV-Headers_SOURCE_DIR})
else()
  if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Headers)
    set(SPIRV_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV-Headers)
  else()
    set(SPIRV_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spirv-headers)
  endif()
endif()

if (IS_DIRECTORY ${SPIRV_HEADER_DIR})
  # TODO(dneto): We should not be modifying the parent scope.
  set(SPIRV_HEADER_INCLUDE_DIR ${SPIRV_HEADER_DIR}/include PARENT_SCOPE)

  # Add SPIRV-Headers as a sub-project if it isn't already defined.
  # Do this so enclosing projects can use SPIRV-Headers_SOURCE_DIR to find
  # headers to include.
  if (NOT DEFINED SPIRV-Headers_SOURCE_DIR)
    set(SPIRV_HEADERS_SKIP_INSTALL ON)
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
    add_subdirectory(${SPIRV_HEADER_DIR})
  endif()
else()
  message(FATAL_ERROR
    "SPIRV-Headers was not found - please checkout a copy under external/.")
endif()

if (NOT ${SPIRV_SKIP_TESTS})
  # Find gmock if we can. If it's not already configured, then try finding
  # it in external/googletest.
  if (TARGET gmock)
    message(STATUS "Google Mock already configured")
  else()
    set(GMOCK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    if(EXISTS ${GMOCK_DIR})
      if(MSVC)
        # Our tests use ::testing::Combine.  Work around a compiler
        # detection problem in googletest, where that template is
        # accidentally disabled for VS 2017.
        # See https://github.com/google/googletest/issues/1352
        add_definitions(-DGTEST_HAS_COMBINE=1)
      endif()
      if(WIN32)
        option(gtest_force_shared_crt
          "Use shared (DLL) run-time lib even when Google Test is built as static lib."
          ON)
      endif()
      # gtest requires special defines for building as a shared
      # library, simply always build as static.
      push_variable(BUILD_SHARED_LIBS 0)
      add_subdirectory(${GMOCK_DIR} EXCLUDE_FROM_ALL)
      pop_variable(BUILD_SHARED_LIBS)
    endif()
  endif()
  if (TARGET gmock)
    set(GTEST_TARGETS
      gtest
      gtest_main
      gmock
      gmock_main
    )
    foreach(target ${GTEST_TARGETS})
      set_property(TARGET ${target} PROPERTY FOLDER GoogleTest)
    endforeach()
  endif()

  # Find Effcee and RE2, for testing.

  # First find RE2, since Effcee depends on it.
  # If already configured, then use that.  Otherwise, prefer to find it under 're2'
  # in this directory.
  if (NOT TARGET re2)
    # If we are configuring RE2, then turn off its testing.  It takes a long time and
    # does not add much value for us.  If an enclosing project configured RE2, then it
    # has already chosen whether to enable RE2 testing.
    set(RE2_BUILD_TESTING OFF CACHE STRING "Run RE2 Tests")
    if (NOT RE2_SOURCE_DIR)
      if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/re2)
        set(RE2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/re2" CACHE STRING "RE2 source dir" )
      endif()
    endif()
  endif()

  if (NOT TARGET effcee)
    # Expect to find effcee in this directory.
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/effcee)
      # If we're configuring RE2 (via Effcee), then turn off RE2 testing.
      if (NOT TARGET re2)
        set(RE2_BUILD_TESTING OFF)
      endif()
      if (MSVC)
        # SPIRV-Tools uses the shared CRT with MSVC.  Tell Effcee to do the same.
        set(EFFCEE_ENABLE_SHARED_CRT ON)
      endif()
      set(EFFCEE_BUILD_SAMPLES OFF CACHE BOOL "Do not build Effcee examples")
      if (NOT TARGET effcee)
        set(EFFCEE_BUILD_TESTING OFF CACHE BOOL "Do not build Effcee test suite")
      endif()
      push_variable(BUILD_SHARED_LIBS 0) # effcee does not export any symbols for building as a DLL. Always build as static.
      add_subdirectory(effcee EXCLUDE_FROM_ALL)
      pop_variable(BUILD_SHARED_LIBS)
      set_property(TARGET effcee PROPERTY FOLDER Effcee)
      # Turn off warnings for effcee and re2
      set_property(TARGET effcee APPEND PROPERTY COMPILE_OPTIONS -w)
      set_property(TARGET re2 APPEND PROPERTY COMPILE_OPTIONS -w)
    endif()
  endif()
endif()

if(SPIRV_BUILD_FUZZER)

  function(backup_compile_options)
    get_property(
        SPIRV_TOOLS_BACKUP_EXTERNAL_COMPILE_OPTIONS
        DIRECTORY
        PROPERTY COMPILE_OPTIONS
        )
  endfunction()

  function(restore_compile_options)
    set_property(
        DIRECTORY
        PROPERTY COMPILE_OPTIONS
        ${SPIRV_TOOLS_BACKUP_EXTERNAL_COMPILE_OPTIONS}
    )
  endfunction()

  if(NOT TARGET protobuf::libprotobuf OR NOT TARGET protobuf::protoc)

    set(SPIRV_TOOLS_PROTOBUF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/cmake)
    if (NOT IS_DIRECTORY ${SPIRV_TOOLS_PROTOBUF_DIR})
      message(
          FATAL_ERROR
          "protobuf not found - please checkout a copy under external/.")
    endif()
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests")
    set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Do not build protobuf static runtime")

    backup_compile_options()

    if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
      add_compile_options(-Wno-inconsistent-missing-override)
    endif()

    add_subdirectory(${SPIRV_TOOLS_PROTOBUF_DIR} EXCLUDE_FROM_ALL)

    restore_compile_options()

  endif()
endif()
