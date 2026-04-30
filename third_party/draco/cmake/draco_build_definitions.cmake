# Copyright 2021 The Draco Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

if(DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_
set(DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_ 1)

# Utility for controlling the main draco library dependency. This changes in
# shared builds, and when an optional target requires a shared library build.
macro(set_draco_target)
  if(MSVC)
    set(draco_dependency draco)
    set(draco_plugin_dependency ${draco_dependency})
  else()
    if(BUILD_SHARED_LIBS)
      set(draco_dependency draco_shared)
    else()
      set(draco_dependency draco_static)
    endif()
    set(draco_plugin_dependency draco_static)
  endif()
endmacro()

# Configures flags and sets build system globals.
macro(draco_set_build_definitions)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lowercase)

  if(build_type_lowercase MATCHES "rel" AND DRACO_FAST)
    if(MSVC)
      list(APPEND draco_msvc_cxx_flags "/Ox")
    else()
      list(APPEND draco_base_cxx_flags "-O3")
    endif()
  endif()

  draco_load_version_info()

  # Library version info. See the libtool docs for updating the values:
  # https://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
  #
  # c=<current>, r=<revision>, a=<age>
  #
  # libtool generates a .so file as .so.[c-a].a.r, while -version-info c:r:a is
  # passed to libtool.
  #
  # We set DRACO_SOVERSION = [c-a].a.r
  set(LT_CURRENT 9)
  set(LT_REVISION 0)
  set(LT_AGE 0)
  math(EXPR DRACO_SOVERSION_MAJOR "${LT_CURRENT} - ${LT_AGE}")
  set(DRACO_SOVERSION "${DRACO_SOVERSION_MAJOR}.${LT_AGE}.${LT_REVISION}")
  unset(LT_CURRENT)
  unset(LT_REVISION)
  unset(LT_AGE)

  list(APPEND draco_include_paths "${draco_root}" "${draco_root}/src"
              "${draco_build}")

  if(DRACO_TRANSCODER_SUPPORTED)
    draco_setup_eigen()
    draco_setup_filesystem()
    draco_setup_tinygltf()


  endif()


  list(APPEND draco_defines "DRACO_CMAKE=1"
              "DRACO_FLAGS_SRCDIR=\"${draco_root}\""
              "DRACO_FLAGS_TMPDIR=\"/tmp\"")

  if(MSVC OR WIN32)
    list(APPEND draco_defines "_CRT_SECURE_NO_DEPRECATE=1" "NOMINMAX=1")

    if(BUILD_SHARED_LIBS)
      set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
  endif()

  if(NOT MSVC)
    if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
      # Ensure 64-bit platforms can support large files.
      list(APPEND draco_defines "_LARGEFILE_SOURCE" "_FILE_OFFSET_BITS=64")
    endif()

    if(NOT DRACO_DEBUG_COMPILER_WARNINGS)
      if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND draco_clang_cxx_flags
                    "-Wno-implicit-const-int-float-conversion")
      else()
        list(APPEND draco_base_cxx_flags "-Wno-deprecated-declarations")
      endif()
    endif()
  endif()

  if(ANDROID)
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
      set(CMAKE_ANDROID_ARM_MODE ON)
    endif()
  endif()

  set_draco_target()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6")
      # Quiet warnings in copy-list-initialization where {} elision has always
      # been allowed.
      list(APPEND draco_clang_cxx_flags "-Wno-missing-braces")
    endif()
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7")
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7")
        # Quiet gcc 6 vs 7 abi warnings:
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77728
        list(APPEND draco_base_cxx_flags "-Wno-psabi")
        list(APPEND ABSL_GCC_FLAGS "-Wno-psabi")
      endif()
    endif()
  endif()

  # Source file names ending in these suffixes will have the appropriate
  # compiler flags added to their compile commands to enable intrinsics.
  set(draco_neon_source_file_suffix "neon.cc")
  set(draco_sse4_source_file_suffix "sse4.cc")

  if((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND ${CMAKE_CXX_COMPILER_VERSION}
                                                  VERSION_LESS 5)
     OR (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"
         AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4))
    message(
      WARNING "GNU/GCC < v5 or Clang/LLVM < v4, ENABLING COMPATIBILITY MODE.")
    draco_enable_feature(FEATURE "DRACO_OLD_GCC")
  endif()

  if(EMSCRIPTEN)
    draco_check_emscripten_environment()
    draco_get_required_emscripten_flags(
      FLAG_LIST_VAR_COMPILER draco_base_cxx_flags
      FLAG_LIST_VAR_LINKER draco_base_exe_linker_flags)
  endif()

  draco_configure_sanitizer()
endmacro()
