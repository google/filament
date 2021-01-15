if(DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_)
  return()
endif() # DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_
set(DRACO_CMAKE_DRACO_BUILD_DEFINITIONS_CMAKE_ 1)

# Utility for controlling the main draco library dependency. This changes in
# shared builds, and when an optional target requires a shared library build.
macro(set_draco_target)
  if(MSVC OR WIN32)
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

  if(BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
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
  set(DRACO_SOVERSION 1)

  list(APPEND draco_include_paths "${draco_root}" "${draco_root}/src"
              "${draco_build}")

  if(DRACO_ABSL)
    list(APPEND draco_include_path "${draco_root}/third_party/abseil-cpp")
  endif()


  list(APPEND draco_gtest_include_paths
              "${draco_root}/../googletest/googlemock/include"
              "${draco_root}/../googletest/googlemock"
              "${draco_root}/../googletest/googletest/include"
              "${draco_root}/../googletest/googletest")
  list(APPEND draco_test_include_paths ${draco_include_paths}
              ${draco_gtest_include_paths})
  list(APPEND draco_defines "DRACO_CMAKE=1"
              "DRACO_FLAGS_SRCDIR=\"${draco_root}\""
              "DRACO_FLAGS_TMPDIR=\"/tmp\"")

  if(MSVC OR WIN32)
    list(APPEND draco_defines "_CRT_SECURE_NO_DEPRECATE=1" "NOMINMAX=1")

    if(BUILD_SHARED_LIBS)
      set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
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

  if((${CMAKE_CXX_COMPILER_ID}
      STREQUAL
      "GNU"
      AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 5)
     OR (${CMAKE_CXX_COMPILER_ID}
         STREQUAL
         "Clang"
         AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4))
    message(
      WARNING "GNU/GCC < v5 or Clang/LLVM < v4, ENABLING COMPATIBILITY MODE.")
    draco_enable_feature(FEATURE "DRACO_OLD_GCC")
  endif()

  if(EMSCRIPTEN)
    draco_check_emscripten_environment()
    draco_get_required_emscripten_flags(FLAG_LIST_VAR draco_base_cxx_flags)
  endif()
endmacro()
