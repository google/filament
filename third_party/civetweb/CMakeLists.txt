# Determines what CMake APIs we can rely on
cmake_minimum_required (VERSION 2.8.11)
if (${CMAKE_VERSION} VERSION_GREATER 3.2.2)
  cmake_policy(VERSION 3.2.2)
endif()
if (${CMAKE_VERSION} VERSION_GREATER 3.1 OR
    ${CMAKE_VERSION} VERSION_EQUAL 3.1)
  cmake_policy(SET CMP0054 NEW)
endif()

# Set up the project
project (civetweb)

# Detect the platform reliably
if(ZEPHYR_BASE)
    if (NOT CONFIG_CIVETWEB)
      return()
    endif()
    SET(ZEPHYR YES)
elseif(NOT MACOSX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(DARWIN YES)
elseif(NOT BSD AND ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    SET(FREEBSD YES)
elseif(NOT LINUX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(LINUX YES)
endif()

# Do not allow in source builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make sure we can import out CMake functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Load in the needed CMake modules
include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(AddCCompilerFlag)
include(AddCXXCompilerFlag)
include(DetermineTargetArchitecture)
include(CMakeDependentOption)

set(CIVETWEB_VERSION "1.11.0" CACHE STRING "The version of the civetweb library")
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" CIVETWEB_VERSION_MATCH "${CIVETWEB_VERSION}")
if ("${CIVETWEB_VERSION_MATCH}" STREQUAL "")
  message(FATAL_ERROR "Must specify a semantic version: major.minor.patch")
endif()
set(CIVETWEB_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(CIVETWEB_VERSION_MINOR "${CMAKE_MATCH_2}")
set(CIVETWEB_VERSION_PATCH "${CMAKE_MATCH_3}")
determine_target_architecture(CIVETWEB_ARCHITECTURE)
include(GNUInstallDirs)

# CTest automation
option(CIVETWEB_BUILD_TESTING "Enable automated testing of civetweb" ON)
message(STATUS "Enabling tests in the build - ${CIVETWEB_BUILD_TESTING}")

# C++ wrappers
option(CIVETWEB_ENABLE_THIRD_PARTY_OUTPUT "Shows the output of third party dependency processing" OFF)

# Thread Stack Size
set(CIVETWEB_THREAD_STACK_SIZE 102400 CACHE STRING
  "The stack size in bytes for each thread created")
set_property(CACHE CIVETWEB_THREAD_STACK_SIZE PROPERTY VALUE ${CIVETWEB_THREAD_STACK_SIZE})
message(STATUS "Thread Stack Size - ${CIVETWEB_THREAD_STACK_SIZE}")

option(CIVETWEB_ENABLE_SERVER_EXECUTABLE "Enable building of the server executable" ON)
message(STATUS "Enabling server executable - ${CIVETWEB_ENABLE_SERVER_EXECUTABLE}")

# Serve no files from the web server
option(CIVETWEB_SERVE_NO_FILES "Configures the server to serve no static files" OFF)
message(STATUS "Serve no static files - ${CIVETWEB_SERVE_NO_FILES}")

# Serve no files from the web server
option(CIVETWEB_DISABLE_CGI "Disables CGI, so theserver will not execute CGI scripts" OFF)
message(STATUS "Disable CGI support - ${CIVETWEB_DISABLE_CGI}")

# Disable caching
option(CIVETWEB_DISABLE_CACHING "Disables caching, so that no timegm is used." OFF)
message(STATUS "Disable caching support - ${CIVETWEB_DISABLE_CACHING}")

# C++ wrappers
option(CIVETWEB_ENABLE_CXX "Enables the C++ wrapper library" OFF)
message(STATUS "C++ wrappers - ${CIVETWEB_ENABLE_CXX}")

# IP Version 6
option(CIVETWEB_ENABLE_IPV6 "Enables the IP version 6 support" OFF)
message(STATUS "IP Version 6 - ${CIVETWEB_ENABLE_IPV6}")

# Websocket support
option(CIVETWEB_ENABLE_WEBSOCKETS "Enable websockets connections" OFF)
message(STATUS "Websockets support - ${CIVETWEB_ENABLE_WEBSOCKETS}")

# Server statistics support
option(CIVETWEB_ENABLE_SERVER_STATS "Enable server statistics" OFF)
message(STATUS "Server statistics support - ${CIVETWEB_ENABLE_SERVER_STATS}")

# Memory debugging
option(CIVETWEB_ENABLE_MEMORY_DEBUGGING "Enable the memory debugging features" OFF)
message(STATUS "Memory Debugging - ${CIVETWEB_ENABLE_MEMORY_DEBUGGING}")

# ASAN in debug mode (-fsanitize=address, etc)
option(CIVETWEB_ENABLE_ASAN "Enable ASAN in debug mode" ON)
message(STATUS "ASAN in debug mode - ${CIVETWEB_ENABLE_ASAN}")

# ARCH flag
option(CIVETWEB_ARCH "Force 32/64 bit architecture" OFF)
message(STATUS "Force x32 / x64 architecture - ${CIVETWEB_ARCH}")

# LUA CGI support
option(CIVETWEB_ENABLE_LUA "Enable Lua CGIs" OFF)
message(STATUS "Lua CGI support - ${CIVETWEB_ENABLE_LUA}")

# Enable installing CivetWeb executables
option(CIVETWEB_INSTALL_EXECUTABLE "Enable installing CivetWeb executable" ON)
mark_as_advanced(FORCE CIVETWEB_INSTALL_EXECUTABLE) # Advanced users can disable
message(STATUS "Executable installation - ${CIVETWEB_INSTALL_EXECUTABLE}")

# Allow builds to complete with warnings (do not set -Werror)
# CivetWeb Linux support is stable:
# Builds for GCC 4.6 and clang 3.4 are free from warnings.
# However, GCC introduced a couple of new, partially idiotic warnings,
# that can not be disabled using a #pragma directive.
# It seems unreasonable to have all GCC versions warning free, but only
# some selected ones.
option(CIVETWEB_ALLOW_WARNINGS "Do not stop build if there are warnings" ON)
message(STATUS "Build if there are warnings - ${CIVETWEB_ALLOW_WARNINGS}")

if (NOT CIVETWEB_ALLOW_WARNINGS)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(FATAL_ERROR "Cannot compile with warning as errors, until this GCC bug is solved: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431")
  endif()
endif()

# Link to the shared LUA library
cmake_dependent_option(
  CIVETWEB_ENABLE_LUA_SHARED  "Link to the shared LUA system library" OFF
 CIVETWEB_ENABLE_LUA OFF)
if (CIVETWEB_ENABLE_LUA)
  message(STATUS "Linking shared Lua library - ${CIVETWEB_ENABLE_LUA_SHARED}")
endif()

# Lua Third Party Settings
if (CIVETWEB_ENABLE_LUA)
  if (NOT CIVETWEB_ENABLE_LUA_SHARED)
    # Lua Version
    set(CIVETWEB_LUA_VERSION 5.2.4 CACHE STRING
      "The version of Lua to build and include statically")
    set_property(CACHE CIVETWEB_LUA_VERSION PROPERTY VALUE ${CIVETWEB_LUA_VERSION})
    message(STATUS "Lua Version - ${CIVETWEB_LUA_VERSION}")
    mark_as_advanced(CIVETWEB_LUA_VERSION)

    # Lua Verification Hash
    set(CIVETWEB_LUA_MD5_HASH 913fdb32207046b273fdb17aad70be13 CACHE STRING
      "The hash of Lua archive to be downloaded")
    set_property(CACHE CIVETWEB_LUA_MD5_HASH PROPERTY VALUE ${CIVETWEB_LUA_MD5_HASH})
    mark_as_advanced(CIVETWEB_LUA_MD5_HASH)
  endif()

  # Lua Filesystem Version
  set(CIVETWEB_LUA_FILESYSTEM_VERSION 1.6.3 CACHE STRING
    "The version of Lua Filesystem to build and include statically")
  set_property(CACHE CIVETWEB_LUA_FILESYSTEM_VERSION PROPERTY VALUE ${CIVETWEB_LUA_FILESYSTEM_VERSION})
  message(STATUS "Lua Filesystem Version - ${CIVETWEB_LUA_FILESYSTEM_VERSION}")
  mark_as_advanced(CIVETWEB_LUA_FILESYSTEM_VERSION)

  # Lua Filesystem Verification Hash
  set(CIVETWEB_LUA_FILESYSTEM_MD5_HASH d0552c7e5a082f5bb2865af63fb9dc95 CACHE STRING
    "The hash of Lua Filesystem archive to be downloaded")
  set_property(CACHE CIVETWEB_LUA_FILESYSTEM_MD5_HASH PROPERTY VALUE ${CIVETWEB_LUA_FILESYSTEM_MD5_HASH})
  mark_as_advanced(CIVETWEB_LUA_FILESYSTEM_MD5_HASH)

  # Lua SQLite Version
  set(CIVETWEB_LUA_SQLITE_VERSION 0.9.3 CACHE STRING
    "The version of Lua SQLite to build and include statically")
  set_property(CACHE CIVETWEB_LUA_SQLITE_VERSION PROPERTY VALUE ${CIVETWEB_LUA_SQLITE_VERSION})
  message(STATUS "Lua SQLite Version - ${CIVETWEB_LUA_SQLITE_VERSION}")
  mark_as_advanced(CIVETWEB_LUA_SQLITE_VERSION)

  # Lua SQLite Verification Hash
  set(CIVETWEB_LUA_SQLITE_MD5_HASH 43234ae08197dfce6da02482ed14ec92 CACHE STRING
    "The hash of Lua SQLite archive to be downloaded")
  set_property(CACHE CIVETWEB_LUA_SQLITE_MD5_HASH PROPERTY VALUE ${CIVETWEB_LUA_SQLITE_MD5_HASH})
  mark_as_advanced(CIVETWEB_LUA_SQLITE_MD5_HASH)

  # Lua XML Version
  set(CIVETWEB_LUA_XML_VERSION 1.8.0 CACHE STRING
    "The version of Lua XML to build and include statically")
  set_property(CACHE CIVETWEB_LUA_XML_VERSION PROPERTY VALUE ${CIVETWEB_LUA_XML_VERSION})
  message(STATUS "Lua XML Version - ${CIVETWEB_LUA_XML_VERSION}")
  mark_as_advanced(CIVETWEB_LUA_XML_VERSION)

  # Lua XML Verification Hash
  set(CIVETWEB_LUA_XML_MD5_HASH 25e4c276c5d8716af1de0c7853aec2b4 CACHE STRING
    "The hash of Lua XML archive to be downloaded")
  set_property(CACHE CIVETWEB_LUA_XML_MD5_HASH PROPERTY VALUE ${CIVETWEB_LUA_XML_MD5_HASH})
  mark_as_advanced(CIVETWEB_LUA_XML_MD5_HASH)

  # SQLite Version
  set(CIVETWEB_SQLITE_VERSION 3.8.9 CACHE STRING
    "The version of SQLite to build and include statically")
  set_property(CACHE CIVETWEB_SQLITE_VERSION PROPERTY VALUE ${CIVETWEB_SQLITE_VERSION})
  message(STATUS "SQLite Version - ${CIVETWEB_SQLITE_VERSION}")
  mark_as_advanced(CIVETWEB_SQLITE_VERSION)

  # SQLite Verification Hash
  set(CIVETWEB_SQLITE_MD5_HASH 02e9c3a6daa8b8587cf6bef828c2e33f CACHE STRING
    "The hash of SQLite archive to be downloaded")
  set_property(CACHE CIVETWEB_SQLITE_MD5_HASH PROPERTY VALUE ${CIVETWEB_SQLITE_MD5_HASH})
  mark_as_advanced(CIVETWEB_SQLITE_MD5_HASH)
endif()

# Duktape CGI support
option(CIVETWEB_ENABLE_DUKTAPE "Enable Duktape CGIs" OFF)
message(STATUS "Duktape CGI support - ${CIVETWEB_ENABLE_DUKTAPE}")

# SSL support
option(CIVETWEB_ENABLE_SSL "Enables the secure socket layer" ON)
message(STATUS "SSL support - ${CIVETWEB_ENABLE_SSL}")

# OpenSSL 1.1 API
option(CIVETWEB_SSL_OPENSSL_API_1_1 "Use the OpenSSL 1.1 API" OFF)
message(STATUS "Compile for OpenSSL 1.1 API - ${CIVETWEB_SSL_OPENSSL_API_1_1}")

# Dynamically load or link the SSL libraries
cmake_dependent_option(
  CIVETWEB_ENABLE_SSL_DYNAMIC_LOADING "Dynamically loads the SSL library rather than linking it" ON
  CIVETWEB_ENABLE_SSL OFF)
if (CIVETWEB_ENABLE_SSL)
  message(STATUS "Dynamically load SSL libraries - ${CIVETWEB_ENABLE_SSL_DYNAMIC_LOADING}")
endif()

# Link time optimization
option(CIVETWEB_ENABLE_LTO "Enable link time optimization" OFF)
if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${BUILD_SHARED_LIBS}" STREQUAL "SHARED" OR
    ${CMAKE_CXX_COMPILER_VERSION} GREATER 5)
  option(CIVETWEB_CXX_ENABLE_LTO "Enable link time optimization for the C++ wrapper library" ON)
else()
  option(CIVETWEB_CXX_ENABLE_LTO "Enable link time optimization for the C++ wrapper library" OFF)
endif()

# Third Party Download location
set(CIVETWEB_THIRD_PARTY_DIR "${CMAKE_BINARY_DIR}/third_party" CACHE STRING
  "The location that third party code is downloaded, built and installed")
set_property(CACHE CIVETWEB_THIRD_PARTY_DIR PROPERTY VALUE ${CIVETWEB_THIRD_PARTY_DIR})

# Unix systems can define the dynamic library names to load
if (CIVETWEB_ENABLE_SSL_DYNAMIC_LOADING AND NOT DARWIN AND UNIX)
  # SSL library name
  set(CIVETWEB_SSL_SSL_LIB "libssl.so" CACHE STRING
    "The name of the SSL library to load")
  set_property(CACHE CIVETWEB_SSL_SSL_LIB PROPERTY VALUE ${CIVETWEB_SSL_SSL_LIB})
  message(STATUS "SSL Library Name - ${CIVETWEB_SSL_SSL_LIB}")

  # Crytography library name
  set(CIVETWEB_SSL_CRYPTO_LIB "libcrypto.so" CACHE STRING
    "The name of the SSL Cryptography library to load")
  set_property(CACHE CIVETWEB_SSL_CRYPTO_LIB PROPERTY VALUE ${CIVETWEB_SSL_CRYPTO_LIB})
  message(STATUS "SSL Cryptography Library Name - ${CIVETWEB_SSL_CRYPTO_LIB}")
endif()

# Allow warnings in 3rd party components
if (CIVETWEB_ENABLE_LUA OR CIVETWEB_ENABLE_DUKTAPE)
SET(CIVETWEB_ALLOW_WARNINGS YES)
endif()

# The C and C++ standards to use
set(CIVETWEB_C_STANDARD auto CACHE STRING
  "The C standard to use; auto determines the latest supported by the compiler")
set_property(CACHE CIVETWEB_C_STANDARD PROPERTY STRINGS auto c11 c99 c89)
set(CIVETWEB_CXX_STANDARD auto CACHE STRING
  "The C++ standard to use; auto determines the latest supported by the compiler")
set_property(CACHE CIVETWEB_CXX_STANDARD PROPERTY STRINGS auto c++14 c++11 c++98)

# Configure the linker
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  find_program(GCC_AR gcc-ar)
  if (GCC_AR)
    set(CMAKE_AR ${GCC_AR})
  endif()
  find_program(GCC_RANLIB gcc-ranlib)
  if (GCC_RANLIB)
    set(CMAKE_RANLIB ${GCC_RANLIB})
  endif()
endif()

# Configure the C compiler
message(STATUS "Configuring C Compiler")
if ("${CIVETWEB_C_STANDARD}" STREQUAL "auto")
  add_c_compiler_flag(-std=c11)
  if (NOT HAVE_C_FLAG_STD_C11)
    add_c_compiler_flag(-std=c99)
    if (NOT HAVE_C_FLAG_STD_C99)
      add_c_compiler_flag(-std=c89)
    endif()
  endif()
else()
  add_c_compiler_flag(-std=${CIVETWEB_C_STANDARD})
endif()

if (MINGW)
  add_c_compiler_flag(-Wno-format)
endif()
if (NOT CIVETWEB_ALLOW_WARNINGS)
  add_c_compiler_flag(-Werror)
  add_c_compiler_flag(/WX)
endif()
if (${CIVETWEB_ENABLE_LTO})
  add_c_compiler_flag(-flto RELEASE)
endif()
if (${CIVETWEB_ENABLE_ASAN})
add_c_compiler_flag(-fsanitize=undefined DEBUG)
add_c_compiler_flag(-fsanitize=address DEBUG)
if (HAVE_C_FLAG_FSANITIZE_ADDRESS)
  add_c_compiler_flag(-static-asan DEBUG)
endif()
endif()
if (MINGW)
  add_c_compiler_flag(-mwindows)
endif()

# Coverage build type
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the C compiler during coverage builds."
    FORCE)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used for linking binaries during coverage builds."
    FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}" CACHE STRING
    "Flags used by the shared libraries linker during coverage builds."
    FORCE)
mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
    FORCE)
add_c_compiler_flag(--coverage COVERAGE)

# Configure the C++ compiler
if (CIVETWEB_ENABLE_CXX)
  message(STATUS "Configuring C++ Compiler")
  if ("${CIVETWEB_CXX_STANDARD}" STREQUAL "auto")
    add_cxx_compiler_flag(-std=c++14)
    if (NOT HAVE_CXX_FLAG_STD_CXX14)
      add_cxx_compiler_flag(-std=c++11)
      if (NOT HAVE_CXX_FLAG_STD_CXX11)
        add_cxx_compiler_flag(-std=c++98)
      endif()
    endif()
  else()
    add_cxx_compiler_flag(-std=${CIVETWEB_CXX_STANDARD})
  endif()
  add_cxx_compiler_flag(-Wall)
  add_cxx_compiler_flag(-Wextra)
  add_cxx_compiler_flag(-Wshadow)
  add_cxx_compiler_flag(-Wmissing-prototypes)
  add_cxx_compiler_flag(-Weverything)
  add_cxx_compiler_flag(/W4)
  add_cxx_compiler_flag(-Wno-padded)
  add_cxx_compiler_flag(/Wd4820) # padding
  add_cxx_compiler_flag(-Wno-unused-macros)
  add_cxx_compiler_flag(-Wno-format-nonliteral)
  if (MINGW)
    add_cxx_compiler_flag(-Wno-format)
  endif()
  if (NOT CIVETWEB_ALLOW_WARNINGS)
    add_cxx_compiler_flag(-Werror)
    add_cxx_compiler_flag(/WX)
  endif()
  add_cxx_compiler_flag(-pedantic-errors)
  add_cxx_compiler_flag(-fvisibility=hidden)
  add_cxx_compiler_flag(-fstack-protector-strong RELEASE)

  if (${CIVETWEB_CXX_ENABLE_LTO})
    add_cxx_compiler_flag(-flto RELEASE)
  endif()
  if (${CIVETWEB_ENABLE_ASAN})
  add_cxx_compiler_flag(-fsanitize=undefined DEBUG)
  add_cxx_compiler_flag(-fsanitize=address DEBUG)
  if (HAVE_CXX_FLAG_FSANITIZE_ADDRESS)
    add_cxx_compiler_flag(-static-asan DEBUG)
  endif()
  endif()
  add_cxx_compiler_flag(-fstack-protector-all DEBUG)
  if (MINGW)
    add_cxx_compiler_flag(-mwindows)
  endif()
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING
      "Flags used by the C++ compiler during coverage builds."
      FORCE)
  add_cxx_compiler_flag(--coverage COVERAGE)
endif()

if (NOT ZEPHYR)
  #Warnings: enable everything
  add_c_compiler_flag(-Wall)
  add_c_compiler_flag(-Wextra)
  add_c_compiler_flag(-Wshadow)
  add_c_compiler_flag(-Wconversion)
  add_c_compiler_flag(-Wmissing-prototypes)
  add_c_compiler_flag(-Weverything)
  add_c_compiler_flag(-Wparentheses)
  add_c_compiler_flag(/W4) # VisualStudio highest warning level

  #Warnings: Disable some warnings
  add_c_compiler_flag(-Wno-padded) # padding in structures by compiler
  add_c_compiler_flag(-Wno-unused-macros) # so what?
  Check_C_Compiler_Flag( HAVE_NO_RESERVED_ID_MACROS -Wno-reserved-id-macros)
  if (HAVE_NO_RESERVED_ID_MACROS)
  add_c_compiler_flag(-Wno-reserved-id-macros) # for system headers
  endif (HAVE_NO_RESERVED_ID_MACROS)
  add_c_compiler_flag(-Wno-format-nonliteral) # printf(myFormatStringVar, ...)
  add_c_compiler_flag(-Wno-cast-qual) # const cast
  add_c_compiler_flag(/Wd4820) # padding

  add_c_compiler_flag(-pedantic-errors)
  add_c_compiler_flag(-fvisibility=hidden)
  add_c_compiler_flag(-fstack-protector-strong RELEASE)
  add_c_compiler_flag(-fstack-protector-all DEBUG)
else()
  # This policy is needed to override options with variables
  cmake_policy(SET CMP0077 NEW)

  # Configure what you need/support in Zephyr
  set(CIVETWEB_SERVE_NO_FILES ON)
  set(CIVETWEB_SERVE_NO_FILESYSTEMS ON)
  set(CIVETWEB_DISABLE_CGI ON)
  set(CIVETWEB_DISABLE_CACHING ON)
  set(CIVETWEB_ENABLE_SSL OFF)
  set(CIVETWEB_ENABLE_SSL_DYNAMIC_LOADING OFF)

  set(CIVETWEB_ENABLE_LUA OFF)
  set(CIVETWEB_ENABLE_DUKTAPE OFF)
  set(CIVETWEB_ENABLE_MEMORY_DEBUGGING OFF)
  set(CIVETWEB_ENABLE_SERVER_EXECUTABLE OFF)
  set(CIVETWEB_ENABLE_ASAN OFF)
  set(CIVETWEB_INSTALL_EXECUTABLE OFF)

  set(CIVETWEB_THREAD_STACK_SIZE 0)

  set(BUILD_SHARED_LIBS OFF)

  add_definitions(-DMG_EXTERNAL_FUNCTION_mg_cry_internal_impl)
  add_definitions(-DMG_EXTERNAL_FUNCTION_log_access)

  add_definitions(-DNO_ALTERNATIVE_QUEUE)
  add_definitions(-DZEPHYR_VERSION=${KERNEL_VERSION_STRING})

  zephyr_interface_library_named(CIVETWEB)

  target_include_directories(CIVETWEB INTERFACE src)
  target_include_directories(CIVETWEB INTERFACE include)
  target_include_directories(CIVETWEB INTERFACE ${CMAKE_SOURCE_DIR}/src)

  zephyr_include_directories(include)

  zephyr_library()
  zephyr_library_sources(
    src/civetweb.c
  )

  zephyr_library_link_libraries(CIVETWEB)
  target_link_libraries(CIVETWEB INTERFACE zephyr_interface)
endif()


# Set up the definitions
if (${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug")
  add_definitions(-DDEBUG)
  add_definitions(-O0)
  add_definitions(-g)
endif()
if (CIVETWEB_ENABLE_IPV6)
  add_definitions(-DUSE_IPV6)
endif()
if (CIVETWEB_ENABLE_WEBSOCKETS)
  add_definitions(-DUSE_WEBSOCKET)
endif()
if (CIVETWEB_ENABLE_SERVER_STATS)
  add_definitions(-DUSE_SERVER_STATS)
endif()
if (CIVETWEB_SERVE_NO_FILES)
  add_definitions(-DNO_FILES)
endif()
if (CIVETWEB_SERVE_NO_FILESYSTEMS)
  add_definitions(-DNO_FILESYSTEMS)
endif()
if (CIVETWEB_DISABLE_CGI)
  add_definitions(-DNO_CGI)
endif()
if (CIVETWEB_DISABLE_CACHING)
  add_definitions(-DNO_CACHING)
endif()
if (CIVETWEB_ENABLE_LUA)
  add_definitions(-DUSE_LUA)
endif()
if (CIVETWEB_ENABLE_DUKTAPE)
  add_definitions(-DUSE_DUKTAPE)
endif()
if (CIVETWEB_ENABLE_MEMORY_DEBUGGING)
  add_definitions(-DMEMORY_DEBUGGING)
endif()
if (NOT CIVETWEB_ENABLE_SSL)
  add_definitions(-DNO_SSL)
elseif (NOT CIVETWEB_ENABLE_SSL_DYNAMIC_LOADING)
  add_definitions(-DNO_SSL_DL)
else()
  if(CIVETWEB_SSL_SSL_LIB)
    add_definitions(-DSSL_LIB="${CIVETWEB_SSL_SSL_LIB}")
  endif()
  if(CIVETWEB_SSL_CRYPTO_LIB)
    add_definitions(-DCRYPTO_LIB="${CIVETWEB_SSL_CRYPTO_LIB}")
  endif()
endif()
if(CIVETWEB_SSL_OPENSSL_API_1_1)
  add_definitions(-DOPENSSL_API_1_1)
endif()
add_definitions(-DUSE_STACK_SIZE=${CIVETWEB_THREAD_STACK_SIZE})

# Set 32 or 64 bit environment
if (${CMAKE_ARCH} MATCHES "[Xx]86")
add_c_compiler_flag(-m32)
endif()
if (${CMAKE_ARCH} MATCHES "[Xx]64")
add_c_compiler_flag(-m64)
endif()
# TODO: add support for -march

if (ZEPHYR)
  return()
endif()

# Build the targets
add_subdirectory(src)

# Enable the testing of the library/executable
include(CTest)
if (CIVETWEB_BUILD_TESTING)
  # Check unit testing framework Version
  set(CIVETWEB_CHECK_VERSION 0.11.0 CACHE STRING
    "The version of Check unit testing framework to build and include statically")
  set_property(CACHE CIVETWEB_CHECK_VERSION PROPERTY VALUE ${CIVETWEB_CHECK_VERSION})
  message(STATUS "Check Unit Testing Framework Version - ${CIVETWEB_CHECK_VERSION}")
  mark_as_advanced(CIVETWEB_CHECK_VERSION)

  # Check unit testing framework Verification Hash
  # Hash for Check 0.10.0: 67a34c40b5bc888737f4e5ae82e9939f
  # Hash for Check 0.11.0: 1b14ee307dca8e954a8219c34484d7c4
  set(CIVETWEB_CHECK_MD5_HASH 1b14ee307dca8e954a8219c34484d7c4 CACHE STRING
    "The hash of Check unit testing framework archive to be downloaded")
  set_property(CACHE CIVETWEB_CHECK_MD5_HASH PROPERTY VALUE ${CIVETWEB_CHECK_MD5_HASH})
  mark_as_advanced(CIVETWEB_CHECK_MD5_HASH)

  # Build the testing
  add_subdirectory(unittest)
endif()

# cmake config file

include(CMakePackageConfigHelpers)

install(
  EXPORT ${PROJECT_NAME}-targets
  NAMESPACE ${PROJECT_NAME}::
  FILE ${PROJECT_NAME}-targets.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  COMPONENT civetweb-cmake-config
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
  ${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
  VERSION ${CIVETWEB_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  COMPONENT civetweb-cmake-config
)

# Set up CPack
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_VENDOR "civetweb Contributors")
set(CPACK_PACKAGE_CONTACT "civetweb@users.noreply.github.com")
set(CPACK_PACKAGE_VERSION_MAJOR "${CIVETWEB_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CIVETWEB_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CIVETWEB_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A HTTP library and server")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_PACKAGE_DEPENDS "openssl")
if (CIVETWEB_ENABLE_LUA_SHARED)
  set(CPACK_PACKAGE_DEPENDS "lua, ${CPACK_PACKAGE_DEPENDS}")
endif()

# RPM Packaging
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "${CIVETWEB_ARCHITECTURE}")
set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_PACKAGE_DEPENDS}")

# Debian Packaging
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CIVETWEB_ARCHITECTURE}")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/civetweb/civetweb")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_PACKAGE_DEPENDS}")

# WiX Packaging
# TODO: www.cmake.org/cmake/help/v3.0/module/CPackWIX.html

# Finalize CPack settings
include(CPack)

