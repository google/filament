# Open Asset Import Library (assimp)
# ----------------------------------------------------------------------
# Copyright (c) 2006-2019, assimp team
#
# All rights reserved.
#
# Redistribution and use of this software in source and binary forms,
# with or without modification, are permitted provided that the
# following conditions are met:
#
# * Redistributions of source code must retain the above
#   copyright notice, this list of conditions and the
#   following disclaimer.
#
# * Redistributions in binary form must reproduce the above
#   copyright notice, this list of conditions and the
#   following disclaimer in the documentation and/or other
#   materials provided with the distribution.
#
# * Neither the name of the assimp team, nor the names of its
#   contributors may be used to endorse or promote products
#   derived from this software without specific prior
#   written permission of the assimp team.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#----------------------------------------------------------------------
SET(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

# Toggles the use of the hunter package manager
option(HUNTER_ENABLED "Enable Hunter package manager support" OFF)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.176.tar.gz"
    SHA1 "2e9ae973d028660b735ac4c6142725ca36a0048a"
)

IF(HUNTER_ENABLED)
  add_definitions(-DASSIMP_USE_HUNTER)
ENDIF(HUNTER_ENABLED)

PROJECT( Assimp VERSION 5.0.0 )

# All supported options ###############################################

OPTION( BUILD_SHARED_LIBS
  "Build package with shared libraries."
  ON
)

OPTION( BUILD_FRAMEWORK
  "Build package as Mac OS X Framework bundle."
  OFF
)
OPTION( ASSIMP_DOUBLE_PRECISION
  "Set to ON to enable double precision processing"
  OFF
)
OPTION( ASSIMP_OPT_BUILD_PACKAGES
  "Set to ON to generate CPack configuration files and packaging targets"
  OFF
)
OPTION( ASSIMP_ANDROID_JNIIOSYSTEM
  "Android JNI IOSystem support is active"
  OFF
)
OPTION( ASSIMP_NO_EXPORT
  "Disable Assimp's export functionality."
  OFF
)
OPTION( ASSIMP_BUILD_ZLIB
  "Build your own zlib"
  OFF
)
OPTION( ASSIMP_BUILD_ASSIMP_TOOLS
  "If the supplementary tools for Assimp are built in addition to the library."
  ON
)
OPTION ( ASSIMP_BUILD_SAMPLES
  "If the official samples are built as well (needs Glut)."
  OFF
)
OPTION ( ASSIMP_BUILD_TESTS
  "If the test suite for Assimp is built in addition to the library."
  ON
)
OPTION ( ASSIMP_COVERALLS
  "Enable this to measure test coverage."
  OFF
)
OPTION ( ASSIMP_WERROR
  "Treat warnings as errors."
  OFF
)
OPTION ( ASSIMP_ASAN
  "Enable AddressSanitizer."
  OFF
)
OPTION ( ASSIMP_UBSAN
  "Enable Undefined Behavior sanitizer."
  OFF
)
OPTION ( SYSTEM_IRRXML
  "Use system installed Irrlicht/IrrXML library."
  OFF
)
OPTION ( BUILD_DOCS
  "Build documentation using Doxygen."
  OFF
)
OPTION( INJECT_DEBUG_POSTFIX
  "Inject debug postfix in .a/.so/.dll lib names"
  ON
)

OPTION ( IGNORE_GIT_HASH
   "Don't call git to get the hash."
   OFF
)

IF (IOS AND NOT HUNTER_ENABLED)
  IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
  ENDIF (NOT CMAKE_BUILD_TYPE)
  ADD_DEFINITIONS(-DENABLE_BITCODE)
ENDIF (IOS AND NOT HUNTER_ENABLED)

# Use subset of Windows.h
if (WIN32)
  ADD_DEFINITIONS( -DWIN32_LEAN_AND_MEAN )
endif()

IF(MSVC)
  OPTION( ASSIMP_INSTALL_PDB
    "Install MSVC debug files."
    ON
  )
  IF(NOT (MSVC_VERSION LESS 1900))
    # Multibyte character set is deprecated since at least MSVC2015 (possibly earlier)
    ADD_DEFINITIONS( -DUNICODE -D_UNICODE )
  ENDIF()
ENDIF(MSVC)

IF (BUILD_FRAMEWORK)
  SET (BUILD_SHARED_LIBS ON)
  MESSAGE(STATUS "Framework bundle building enabled")
ENDIF(BUILD_FRAMEWORK)

IF(NOT BUILD_SHARED_LIBS)
  MESSAGE(STATUS "Shared libraries disabled")
  SET(LINK_SEARCH_START_STATIC TRUE)
ELSE()
  MESSAGE(STATUS "Shared libraries enabled")
ENDIF(NOT BUILD_SHARED_LIBS)

# Define here the needed parameters
SET (ASSIMP_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET (ASSIMP_VERSION_MINOR ${PROJECT_VERSION_MINOR})
SET (ASSIMP_VERSION_PATCH ${PROJECT_VERSION_PATCH})
SET (ASSIMP_VERSION ${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH})
SET (ASSIMP_SOVERSION 5)

SET( ASSIMP_PACKAGE_VERSION "0" CACHE STRING "the package-specific version used for uploading the sources" )
if(NOT HUNTER_ENABLED)
  # Enable C++11 support globally
  set_property( GLOBAL PROPERTY CXX_STANDARD 11 )
endif()

IF(NOT IGNORE_GIT_HASH)
  # Get the current working branch
  EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  # Get the latest abbreviated commit hash of the working branch
  EXECUTE_PROCESS(
    COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
ENDIF()

IF(NOT GIT_COMMIT_HASH)
  SET(GIT_COMMIT_HASH 0)
ENDIF(NOT GIT_COMMIT_HASH)

IF(ASSIMP_DOUBLE_PRECISION)
    ADD_DEFINITIONS(-DASSIMP_DOUBLE_PRECISION)
ENDIF(ASSIMP_DOUBLE_PRECISION)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/revision.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/revision.h
)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/include/assimp/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/assimp/config.h
)

INCLUDE_DIRECTORIES( BEFORE
  ./
  code/
  include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules" )
SET(LIBASSIMP_COMPONENT     "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}" )
SET(LIBASSIMP-DEV_COMPONENT "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}-dev" )
SET(CPACK_COMPONENTS_ALL assimp-bin ${LIBASSIMP_COMPONENT} ${LIBASSIMP-DEV_COMPONENT} assimp-dev)
SET(ASSIMP_LIBRARY_SUFFIX "" CACHE STRING "Suffix to append to library names")

IF( UNIX )
  # Use GNUInstallDirs for Unix predefined directories
  INCLUDE(GNUInstallDirs)
ENDIF( UNIX )

# Grouped compiler settings
IF ((CMAKE_C_COMPILER_ID MATCHES "GNU") AND NOT CMAKE_COMPILER_IS_MINGW)
  IF(NOT HUNTER_ENABLED)
    SET(CMAKE_CXX_FLAGS "-fPIC -std=c++0x ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  ENDIF()
  # hide all not-exported symbols
  SET(CMAKE_CXX_FLAGS "-g -fvisibility=hidden -fno-strict-aliasing -Wall ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
  SET(LIBSTDC++_LIBRARIES -lstdc++)
ELSEIF(MSVC)
  # enable multi-core compilation with MSVC
  ADD_COMPILE_OPTIONS(/MP)
  ADD_COMPILE_OPTIONS( /bigobj )
  # disable "elements of array '' will be default initialized" warning on MSVC2013
  IF(MSVC12)
    ADD_COMPILE_OPTIONS(/wd4351)
  ENDIF()
  SET(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Ob2 /DEBUG:FULL /Zi")
ELSEIF ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
  IF(NOT HUNTER_ENABLED)
    SET(CMAKE_CXX_FLAGS "-fPIC -std=c++11 ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  ENDIF()
  SET(CMAKE_CXX_FLAGS "-g -fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long ${CMAKE_CXX_FLAGS}" )
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ELSEIF( CMAKE_COMPILER_IS_MINGW )
  IF (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
    message(FATAL_ERROR "MinGW is too old to be supported. Please update MinGW and try again.")
  ELSEIF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.3)
    message(WARNING "MinGW is old, if you experience errors, update MinGW.")
  ENDIF()
  IF(NOT HUNTER_ENABLED)
    SET(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fPIC ${CMAKE_C_FLAGS}")
  ENDIF()
  SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long -Wa,-mbig-obj -O3 ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
  ADD_DEFINITIONS( -U__STRICT_ANSI__ )
ENDIF()

IF ( IOS AND NOT HUNTER_ENABLED)
  IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode -Og")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode -Og")
  ELSE()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode -O3")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode -O3")
    # Experimental for pdb generation
  ENDIF()
ENDIF( IOS AND NOT HUNTER_ENABLED)

IF (ASSIMP_COVERALLS)
  MESSAGE(STATUS "Coveralls enabled")
  INCLUDE(Coveralls)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
ENDIF()

IF (ASSIMP_WERROR)
  MESSAGE(STATUS "Treating warnings as errors")
  IF (MSVC)
    ADD_COMPILE_OPTIONS(/WX)
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  ENDIF()
ENDIF()

IF (ASSIMP_ASAN)
  MESSAGE(STATUS "AddressSanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
ENDIF()

IF (ASSIMP_UBSAN)
  MESSAGE(STATUS "Undefined Behavior sanitizer enabled")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all")
ENDIF()

INCLUDE (FindPkgMacros)
INCLUDE (PrecompiledHeader)

# If this is an in-source build (CMAKE_SOURCE_DIR == CMAKE_BINARY_DIR),
# write the library/executable files to the respective directories in the
# source tree. During an out-of-source build, however, do not litter this
# directory, since that is probably what the user wanted to avoid.
IF ( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
  SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/lib" )
  SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/lib" )
  SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin" )
ENDIF ( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )

# Cache these to allow the user to override them manually.
SET( ASSIMP_LIB_INSTALL_DIR "lib" CACHE STRING
  "Path the built library files are installed to." )
SET( ASSIMP_INCLUDE_INSTALL_DIR "include" CACHE STRING
  "Path the header files are installed to." )
SET( ASSIMP_BIN_INSTALL_DIR "bin" CACHE STRING
  "Path the tool executables are installed to." )

get_cmake_property(is_multi_config GENERATOR_IS_MULTI_CONFIG)

IF (INJECT_DEBUG_POSTFIX AND (is_multi_config OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
  SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Debug Postfix for lib, samples and tools")
ELSE()
  SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Debug Postfix for lib, samples and tools")
ENDIF()

# Only generate this target if no higher-level project already has
IF (NOT TARGET uninstall)
  # add make uninstall capability
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/cmake_uninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)
  ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
ENDIF()

IF(HUNTER_ENABLED)
  set(CONFIG_INSTALL_DIR "lib/cmake/${PROJECT_NAME}")
  set(INCLUDE_INSTALL_DIR "include")

  set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

  # Configuration
  set(VERSION_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
  set(PROJECT_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}Config.cmake")
  set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
  set(NAMESPACE "${PROJECT_NAME}::")

  # Include module with fuction 'write_basic_package_version_file'
  include(CMakePackageConfigHelpers)

  # Note: PROJECT_VERSION is used as a VERSION
  write_basic_package_version_file("${VERSION_CONFIG}" COMPATIBILITY SameMajorVersion)

  # Use variables:
  #   * TARGETS_EXPORT_NAME
  #   * PROJECT_NAME
  configure_package_config_file(
      "cmake/assimp-hunter-config.cmake.in"
      "${PROJECT_CONFIG}"
      INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
  )

  install(
      FILES "${PROJECT_CONFIG}" "${VERSION_CONFIG}"
      DESTINATION "${CONFIG_INSTALL_DIR}"
  )

  install(
      EXPORT "${TARGETS_EXPORT_NAME}"
      NAMESPACE "${NAMESPACE}"
      DESTINATION "${CONFIG_INSTALL_DIR}"
  )
ELSE(HUNTER_ENABLED)
  # cmake configuration files
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config.cmake.in"         "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake" @ONLY IMMEDIATE)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimpTargets.cmake.in"         "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets.cmake" @ONLY IMMEDIATE)
  IF (is_multi_config)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimpTargets-debug.cmake.in"   "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-debug.cmake" @ONLY IMMEDIATE)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimpTargets-release.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-release.cmake" @ONLY IMMEDIATE)
    SET(PACKAGE_TARGETS_FILE "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-debug.cmake" "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-release.cmake")
  ELSEIF (CMAKE_BUILD_TYPE STREQUAL Debug)
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimpTargets-debug.cmake.in"   "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-debug.cmake" @ONLY IMMEDIATE)
    SET(PACKAGE_TARGETS_FILE "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-debug.cmake")
  ELSE()
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimpTargets-release.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-release.cmake" @ONLY IMMEDIATE)
    SET(PACKAGE_TARGETS_FILE "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets-release.cmake")
  ENDIF()
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config-version.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" @ONLY IMMEDIATE)
  #we should generated these scripts after CMake VERSION 3.0.2 using export(EXPORT ...) and write_basic_package_version_file(...)
  INSTALL(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/assimpTargets.cmake"
    ${PACKAGE_TARGETS_FILE}
    DESTINATION "${ASSIMP_LIB_INSTALL_DIR}/cmake/assimp-${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}" COMPONENT ${LIBASSIMP-DEV_COMPONENT})
ENDIF(HUNTER_ENABLED)

FIND_PACKAGE( DirectX )

IF( BUILD_DOCS )
  ADD_SUBDIRECTORY(doc)
ENDIF( BUILD_DOCS )

# Look for system installed irrXML
IF ( SYSTEM_IRRXML )
  FIND_PACKAGE( IrrXML REQUIRED )
ENDIF( SYSTEM_IRRXML )

# Search for external dependencies, and build them from source if not found
# Search for zlib
IF(HUNTER_ENABLED)
  hunter_add_package(ZLIB)
  find_package(ZLIB CONFIG REQUIRED)

  add_definitions(-DASSIMP_BUILD_NO_OWN_ZLIB)
  set(ZLIB_FOUND TRUE)
  set(ZLIB_LIBRARIES ZLIB::zlib)
  set(ASSIMP_BUILD_MINIZIP TRUE)
ELSE(HUNTER_ENABLED)
  IF ( NOT ASSIMP_BUILD_ZLIB )
    FIND_PACKAGE(ZLIB)
  ENDIF( NOT ASSIMP_BUILD_ZLIB )

  IF( NOT ZLIB_FOUND )
    MESSAGE(STATUS "compiling zlib from sources")
    INCLUDE(CheckIncludeFile)
    INCLUDE(CheckTypeSize)
    INCLUDE(CheckFunctionExists)

    # Explicitly turn off ASM686 and AMD64 cmake options.
    # The AMD64 option causes a build failure on MSVC and the ASM builds seem to have problems:
    #		https://github.com/madler/zlib/issues/41#issuecomment-125848075
    # Also prevents these options from "polluting" the cmake options if assimp is being
    # included as a submodule.
    set( ASM686 FALSE CACHE INTERNAL "Override ZLIB flag to turn off assembly" FORCE )
    set( AMD64 FALSE CACHE INTERNAL "Override ZLIB flag to turn off assembly" FORCE )

    # compile from sources
    ADD_SUBDIRECTORY(contrib/zlib)
    SET(ZLIB_FOUND 1)
    SET(ZLIB_LIBRARIES zlibstatic)
    SET(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contrib/zlib ${CMAKE_CURRENT_BINARY_DIR}/contrib/zlib)
    # need to ensure we don't link with system zlib or minizip as well.
    SET(ASSIMP_BUILD_MINIZIP 1)
  ELSE(NOT ZLIB_FOUND)
    ADD_DEFINITIONS(-DASSIMP_BUILD_NO_OWN_ZLIB)
    SET(ZLIB_LIBRARIES_LINKED -lz)
  ENDIF(NOT ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ENDIF(HUNTER_ENABLED)

IF( NOT IOS )
  IF( NOT ASSIMP_BUILD_MINIZIP )
    use_pkgconfig(UNZIP minizip)
  ENDIF( NOT ASSIMP_BUILD_MINIZIP )
ELSE ( NOT IOS )
  IF( NOT BUILD_SHARED_LIBS )
    IF( NOT ASSIMP_BUILD_MINIZIP )
      use_pkgconfig(UNZIP minizip)
    ENDIF( NOT ASSIMP_BUILD_MINIZIP )
  ENDIF ( NOT BUILD_SHARED_LIBS )
ENDIF ( NOT IOS )

IF ( ASSIMP_NO_EXPORT )
  ADD_DEFINITIONS( -DASSIMP_BUILD_NO_EXPORT)
  MESSAGE( STATUS "Build an import-only version of Assimp." )
ENDIF( ASSIMP_NO_EXPORT )

SET ( ASSIMP_BUILD_ARCHITECTURE "" CACHE STRING
  "describe the current architecture."
)
IF    ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
ELSE  ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_ARCHITECTURE="${ASSIMP_BUILD_ARCHITECTURE}"' )
ENDIF ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")

# ${CMAKE_GENERATOR}
SET ( ASSIMP_BUILD_COMPILER "" CACHE STRING
  "describe the current compiler."
)
IF    ( ASSIMP_BUILD_COMPILER STREQUAL "")
ELSE  ( ASSIMP_BUILD_COMPILER STREQUAL "")
  ADD_DEFINITIONS ( -D'ASSIMP_BUILD_COMPILER="${ASSIMP_BUILD_COMPILER}"' )
ENDIF ( ASSIMP_BUILD_COMPILER STREQUAL "")

MARK_AS_ADVANCED ( ASSIMP_BUILD_ARCHITECTURE ASSIMP_BUILD_COMPILER )

SET ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER OFF CACHE BOOL
  "Build the C4D importer, which relies on the non-free Melange SDK."
)

IF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  IF ( MSVC )
    SET(C4D_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Melange/includes")

    # pick the correct prebuilt library
    IF(MSVC15)
      SET(C4D_LIB_POSTFIX "_2017")
    ELSEIF(MSVC14)
      SET(C4D_LIB_POSTFIX "_2015")
    ELSEIF(MSVC12)
      SET(C4D_LIB_POSTFIX "_2013")
    ELSEIF(MSVC11)
      SET(C4D_LIB_POSTFIX "_2012")
    ELSEIF(MSVC10)
      SET(C4D_LIB_POSTFIX "_2010")
    ELSE()
      MESSAGE( FATAL_ERROR
        "C4D is currently only supported with MSVC 10, 11, 12, 14"
      )
    ENDIF()

    SET(C4D_LIB_BASE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Melange/libraries/win")

    SET(C4D_DEBUG_LIBRARIES
      "${C4D_LIB_BASE_PATH}/melangelib${C4D_LIB_POSTFIX}/melangelib_debug.lib"
      "${C4D_LIB_BASE_PATH}/jpeglib${C4D_LIB_POSTFIX}/jpeglib_debug.lib"
    )
    SET(C4D_RELEASE_LIBRARIES
      "${C4D_LIB_BASE_PATH}/melangelib${C4D_LIB_POSTFIX}/melangelib_release.lib"
      "${C4D_LIB_BASE_PATH}/jpeglib${C4D_LIB_POSTFIX}/jpeglib_release.lib"
    )

    # winsock and winmm are necessary dependencies of melange (this is undocumented, but true.)
    SET(C4D_EXTRA_LIBRARIES WSock32.lib Winmm.lib)
  ELSE ()
    MESSAGE( FATAL_ERROR
      "C4D is currently only available on Windows with melange SDK installed in contrib/Melange"
    )
  ENDIF ( MSVC )
ELSE (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)
  ADD_DEFINITIONS( -DASSIMP_BUILD_NO_C4D_IMPORTER )
ENDIF (ASSIMP_BUILD_NONFREE_C4D_IMPORTER)

IF(NOT HUNTER_ENABLED)
  ADD_SUBDIRECTORY(contrib)
ENDIF(NOT HUNTER_ENABLED)

ADD_SUBDIRECTORY( code/ )
IF ( ASSIMP_BUILD_ASSIMP_TOOLS )
  # The viewer for windows only
  IF ( WIN32 AND DirectX_D3DX9_LIBRARY )
    OPTION ( ASSIMP_BUILD_ASSIMP_VIEW "If the Assimp view tool is built. (requires DirectX)" ${DirectX_FOUND} )
    IF ( ASSIMP_BUILD_ASSIMP_VIEW )
      ADD_SUBDIRECTORY( tools/assimp_view/ )
    ENDIF ( ASSIMP_BUILD_ASSIMP_VIEW )
  ENDIF ( WIN32 AND DirectX_D3DX9_LIBRARY )
  # Te command line tool
  ADD_SUBDIRECTORY( tools/assimp_cmd/ )
ENDIF ( ASSIMP_BUILD_ASSIMP_TOOLS )

IF ( ASSIMP_BUILD_SAMPLES)
  IF ( WIN32 )
    ADD_SUBDIRECTORY( samples/SimpleTexturedOpenGL/ )
    ADD_SUBDIRECTORY( samples/SimpleTexturedDirectx11 )
  ENDIF ( WIN32 )
  ADD_SUBDIRECTORY( samples/SimpleOpenGL/ )
ENDIF ( ASSIMP_BUILD_SAMPLES )

IF ( ASSIMP_BUILD_TESTS )
  ADD_SUBDIRECTORY( test/ )
ENDIF ( ASSIMP_BUILD_TESTS )

# Generate a pkg-config .pc for the Assimp library.
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/assimp.pc.in" "${PROJECT_BINARY_DIR}/assimp.pc" @ONLY )
INSTALL( FILES "${PROJECT_BINARY_DIR}/assimp.pc" DESTINATION ${ASSIMP_LIB_INSTALL_DIR}/pkgconfig/ COMPONENT ${LIBASSIMP-DEV_COMPONENT})

IF(CMAKE_CPACK_COMMAND AND UNIX AND ASSIMP_OPT_BUILD_PACKAGES)
  # Packing information
  SET(CPACK_PACKAGE_NAME                    "assimp{ASSIMP_VERSION_MAJOR}.{ASSIMP_VERSION_MINOR}")
  SET(CPACK_PACKAGE_CONTACT "" CACHE STRING "Package maintainer and PGP signer.")
  SET(CPACK_PACKAGE_VENDOR                  "https://github.com/assimp")
  SET(CPACK_PACKAGE_DISPLAY_NAME            "Assimp ${ASSIMP_VERSION}")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY     " - Open Asset Import Library ${ASSIMP_VERSION}")
  SET(CPACK_PACKAGE_VERSION                 "${ASSIMP_VERSION}.${ASSIMP_PACKAGE_VERSION}" )
  SET(CPACK_PACKAGE_VERSION_MAJOR           "${ASSIMP_VERSION_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR           "${ASSIMP_VERSION_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH           "${ASSIMP_VERSION_PATCH}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY       "assimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}")
  SET(CPACK_RESOURCE_FILE_LICENSE           "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

  STRING(TOUPPER ${LIBASSIMP_COMPONENT}     "LIBASSIMP_COMPONENT_UPPER")
  STRING(TOUPPER ${LIBASSIMP-DEV_COMPONENT} "LIBASSIMP-DEV_COMPONENT_UPPER")

  SET(CPACK_COMPONENT_ASSIMP-BIN_DISPLAY_NAME                       "tools")
  SET(CPACK_COMPONENT_ASSIMP-BIN_DEPENDS                            "${LIBASSIMP_COMPONENT}" )
  SET(CPACK_COMPONENT_${LIBASSIMP_COMPONENT_UPPER}_DISPLAY_NAME     "libraries")
  SET(CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT_UPPER}_DISPLAY_NAME "common headers and installs")
  SET(CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT_UPPER}_DEPENDS $    "{LIBASSIMP_COMPONENT}" )
  SET(CPACK_COMPONENT_ASSIMP-DEV_DISPLAY_NAME                       "${CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT}_DISPLAY_NAME}" )
  SET(CPACK_COMPONENT_ASSIMP-DEV_DEPENDS                            "${LIBASSIMP-DEV_COMPONENT}" )
  SET(CPACK_DEBIAN_BUILD_DEPENDS debhelper cmake zlib1g-dev pkg-config)

  # debian
  SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  SET(CPACK_DEBIAN_CMAKE_OPTIONS    "-DBUILD_ASSIMP_SAMPLES:BOOL=${ASSIMP_BUILD_SAMPLES}")
  SET(CPACK_DEBIAN_PACKAGE_SECTION  "libs" )
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS  "${CPACK_COMPONENTS_ALL}")
  SET(CPACK_DEBIAN_PACKAGE_SUGGESTS)
  set(cPACK_DEBIAN_PACKAGE_NAME     "assimp")
  SET(CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES contrib/gtest contrib/zlib workspaces test doc obj samples packaging)
  SET(CPACK_DEBIAN_PACKAGE_SOURCE_COPY svn export --force)
  SET(CPACK_DEBIAN_CHANGELOG)
  execute_process(COMMAND lsb_release -is
    OUTPUT_VARIABLE _lsb_distribution OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _lsb_release_failed)
  SET(CPACK_DEBIAN_DISTRIBUTION_NAME ${_lsb_distribution} CACHE STRING "Name of the distrubiton")
  STRING(TOLOWER ${CPACK_DEBIAN_DISTRIBUTION_NAME} CPACK_DEBIAN_DISTRIBUTION_NAME)
  IF( ${CPACK_DEBIAN_DISTRIBUTION_NAME} STREQUAL "ubuntu" )
    SET(CPACK_DEBIAN_DISTRIBUTION_RELEASES lucid maverick natty oneiric precise CACHE STRING "Release code-names of the distrubiton release")
  ENDIF()
  SET(DPUT_HOST "" CACHE STRING "PPA repository to upload the debian sources")
  INCLUDE(CPack)
  INCLUDE(DebSourcePPA)
ENDIF()

if(WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(BIN_DIR "${PROJECT_SOURCE_DIR}/bin64/")
    SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib64/")
  else()
    SET(BIN_DIR "${PROJECT_SOURCE_DIR}/bin32/")
    SET(LIB_DIR "${PROJECT_SOURCE_DIR}/lib32/")
  ENDIF()

  IF(MSVC_TOOLSET_VERSION)
    set(MSVC_PREFIX "vc${MSVC_TOOLSET_VERSION}")
  ELSE()
    IF(MSVC12)
      SET(ASSIMP_MSVC_VERSION "vc120")
    ELSEIF(MSVC14)
      SET(ASSIMP_MSVC_VERSION "vc140")
    ELSEIF(MSVC15)
      SET(ASSIMP_MSVC_VERSION "vc141")
    ENDIF(MSVC12)
  ENDIF()

  IF(MSVC12 OR MSVC14 OR MSVC15 )
    ADD_CUSTOM_TARGET(UpdateAssimpLibsDebugSymbolsAndDLLs COMMENT "Copying Assimp Libraries ..." VERBATIM)
    IF(CMAKE_GENERATOR MATCHES "^Visual Studio")
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.dll	${BIN_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.dll VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.exp	${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.exp VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.lib	${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.lib VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll		${BIN_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.dll  VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.exp		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.exp VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.ilk		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.ilk VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.lib		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.lib VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb VERBATIM)
    ELSE()
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mt.dll	${BIN_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.dll VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mt.exp	${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.exp VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mt.lib	${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mt.lib VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll		${BIN_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.dll  VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.exp		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.exp VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.ilk		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.ilk VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.lib		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.lib VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb VERBATIM)
      ADD_CUSTOM_COMMAND(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/code/assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb		${LIB_DIR}assimp-${ASSIMP_MSVC_VERSION}-mtd.pdb VERBATIM)
    ENDIF()
  ENDIF(MSVC12 OR MSVC14 OR MSVC15 )
ENDIF (WIN32)
