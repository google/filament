# CMakeLists.txt

# Copyright (c) 2018-2022 Cosmin Truta
# Copyright (c) 2007,2009-2018 Glenn Randers-Pehrson
# Written by Christian Ehrlicher, 2007
# Revised by Roger Lowman, 2009-2010
# Revised by Clifford Yapp, 2011-2012,2017
# Revised by Roger Leigh, 2016
# Revised by Andreas Franek, 2016
# Revised by Sam Serrels, 2017
# Revised by Vadim Barkov, 2017
# Revised by Vicky Pfau, 2018
# Revised by Cameron Cawley, 2018,2021
# Revised by Kyle Bentley, 2018
# Revised by David Callu, 2020
# Revised by Steve Robinson, 2020
# Revised by Simon Hausmann, 2020
# Revised by Alex Gaynor, 2020
# Revised by Owen Rudge, 2020
# Revised by Gleb Mazovetskiy, 2021
# Revised by Christopher Sean Morrison, 2022
# Revised by Martin Storsjo, 2022

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)

project(libpng C ASM)
enable_testing()

set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 6)
set(PNGLIB_REVISION 39)
set(PNGLIB_SUBREVISION 0)
#set(PNGLIB_SUBREVISION "git")
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_REVISION})
set(PNGLIB_SHARED_SOVERSION ${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_SHARED_VERSION ${PNGLIB_SHARED_SOVERSION}.${PNGLIB_REVISION}.${PNGLIB_SUBREVISION})

include(GNUInstallDirs)

# Allow users to specify location of zlib.
# Useful if zlib is being built alongside this as a sub-project.
option(PNG_BUILD_ZLIB "Custom zlib Location, else find_package is used" OFF)

if(NOT PNG_BUILD_ZLIB)
  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

if(UNIX AND NOT APPLE AND NOT BEOS AND NOT HAIKU AND NOT EMSCRIPTEN)
  find_library(M_LIBRARY m)
  if(NOT M_LIBRARY)
    set(M_LIBRARY "")
  endif()
else()
  # libm is not needed and/or not available.
  set(M_LIBRARY "")
endif()

# Public CMake configuration variables.
option(PNG_SHARED "Build shared lib" ON)
option(PNG_STATIC "Build static lib" ON)
option(PNG_EXECUTABLES "Build libpng executables" ON)
option(PNG_TESTS "Build libpng tests" ON)

# Many more configuration options could be added here.
option(PNG_FRAMEWORK "Build OS X framework" OFF)
option(PNG_DEBUG "Build with debug output" OFF)
option(PNG_HARDWARE_OPTIMIZATIONS "Enable hardware optimizations" ON)

set(PNG_PREFIX "" CACHE STRING "Prefix to add to the API function names")
set(DFA_XTRA "" CACHE FILEPATH "File containing extra configuration settings")

# CMake currently sets CMAKE_SYSTEM_PROCESSOR to one of x86_64 or arm64 on macOS,
# based upon the OS architecture, not the target architecture. As such, we need
# to check CMAKE_OSX_ARCHITECTURES to identify which hardware-specific flags to
# enable. Note that this will fail if you attempt to build a universal binary in
# a single CMake invocation.
if (APPLE AND CMAKE_OSX_ARCHITECTURES)
  set(TARGET_ARCH ${CMAKE_OSX_ARCHITECTURES})
else()
  set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(PNG_HARDWARE_OPTIMIZATIONS)

# Set definitions and sources for ARM.
if(TARGET_ARCH MATCHES "^arm" OR
   TARGET_ARCH MATCHES "^aarch64")
  if(TARGET_ARCH MATCHES "^arm64" OR
     TARGET_ARCH MATCHES "^aarch64")
    set(PNG_ARM_NEON_POSSIBLE_VALUES on off)
    set(PNG_ARM_NEON "on"
        CACHE STRING "Enable ARM NEON optimizations: on|off; on is default")
  else()
    set(PNG_ARM_NEON_POSSIBLE_VALUES check on off)
    set(PNG_ARM_NEON "off"
        CACHE STRING "Enable ARM NEON optimizations: check|on|off; off is default")
  endif()
  set_property(CACHE PNG_ARM_NEON
               PROPERTY STRINGS ${PNG_ARM_NEON_POSSIBLE_VALUES})
  list(FIND PNG_ARM_NEON_POSSIBLE_VALUES ${PNG_ARM_NEON} index)
  if(index EQUAL -1)
    message(FATAL_ERROR "PNG_ARM_NEON must be one of [${PNG_ARM_NEON_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_ARM_NEON} STREQUAL "off")
    set(libpng_arm_sources
        arm/arm_init.c
        arm/filter_neon.S
        arm/filter_neon_intrinsics.c
        arm/palette_neon_intrinsics.c)
    if(${PNG_ARM_NEON} STREQUAL "on")
      add_definitions(-DPNG_ARM_NEON_OPT=2)
    elseif(${PNG_ARM_NEON} STREQUAL "check")
      add_definitions(-DPNG_ARM_NEON_CHECK_SUPPORTED)
    endif()
  else()
    add_definitions(-DPNG_ARM_NEON_OPT=0)
  endif()
endif()

# Set definitions and sources for PowerPC.
if(TARGET_ARCH MATCHES "^powerpc*" OR
   TARGET_ARCH MATCHES "^ppc64*")
  set(PNG_POWERPC_VSX_POSSIBLE_VALUES on off)
  set(PNG_POWERPC_VSX "on"
      CACHE STRING "Enable POWERPC VSX optimizations: on|off; on is default")
  set_property(CACHE PNG_POWERPC_VSX
               PROPERTY STRINGS ${PNG_POWERPC_VSX_POSSIBLE_VALUES})
  list(FIND PNG_POWERPC_VSX_POSSIBLE_VALUES ${PNG_POWERPC_VSX} index)
  if(index EQUAL -1)
    message(FATAL_ERROR "PNG_POWERPC_VSX must be one of [${PNG_POWERPC_VSX_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_POWERPC_VSX} STREQUAL "off")
    set(libpng_powerpc_sources
        powerpc/powerpc_init.c
        powerpc/filter_vsx_intrinsics.c)
    if(${PNG_POWERPC_VSX} STREQUAL "on")
      add_definitions(-DPNG_POWERPC_VSX_OPT=2)
    endif()
  else()
    add_definitions(-DPNG_POWERPC_VSX_OPT=0)
  endif()
endif()

# Set definitions and sources for Intel.
if(TARGET_ARCH MATCHES "^i?86" OR
   TARGET_ARCH MATCHES "^x86_64*")
  set(PNG_INTEL_SSE_POSSIBLE_VALUES on off)
  set(PNG_INTEL_SSE "on"
      CACHE STRING "Enable INTEL_SSE optimizations: on|off; on is default")
  set_property(CACHE PNG_INTEL_SSE
               PROPERTY STRINGS ${PNG_INTEL_SSE_POSSIBLE_VALUES})
  list(FIND PNG_INTEL_SSE_POSSIBLE_VALUES ${PNG_INTEL_SSE} index)
  if(index EQUAL -1)
    message(FATAL_ERROR "PNG_INTEL_SSE must be one of [${PNG_INTEL_SSE_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_INTEL_SSE} STREQUAL "off")
    set(libpng_intel_sources
        intel/intel_init.c
        intel/filter_sse2_intrinsics.c)
    if(${PNG_INTEL_SSE} STREQUAL "on")
      add_definitions(-DPNG_INTEL_SSE_OPT=1)
    endif()
  else()
    add_definitions(-DPNG_INTEL_SSE_OPT=0)
  endif()
endif()

# Set definitions and sources for MIPS.
if(TARGET_ARCH MATCHES "mipsel*" OR
   TARGET_ARCH MATCHES "mips64el*")
  set(PNG_MIPS_MSA_POSSIBLE_VALUES on off)
  set(PNG_MIPS_MSA "on"
      CACHE STRING "Enable MIPS_MSA optimizations: on|off; on is default")
  set_property(CACHE PNG_MIPS_MSA
               PROPERTY STRINGS ${PNG_MIPS_MSA_POSSIBLE_VALUES})
  list(FIND PNG_MIPS_MSA_POSSIBLE_VALUES ${PNG_MIPS_MSA} index)
  if(index EQUAL -1)
    message(FATAL_ERROR "PNG_MIPS_MSA must be one of [${PNG_MIPS_MSA_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_MIPS_MSA} STREQUAL "off")
    set(libpng_mips_sources
        mips/mips_init.c
        mips/filter_msa_intrinsics.c)
    if(${PNG_MIPS_MSA} STREQUAL "on")
      add_definitions(-DPNG_MIPS_MSA_OPT=2)
    endif()
  else()
    add_definitions(-DPNG_MIPS_MSA_OPT=0)
  endif()
endif()

else(PNG_HARDWARE_OPTIMIZATIONS)

# Set definitions and sources for ARM.
if(TARGET_ARCH MATCHES "^arm" OR
   TARGET_ARCH MATCHES "^aarch64")
  add_definitions(-DPNG_ARM_NEON_OPT=0)
endif()

# Set definitions and sources for PowerPC.
if(TARGET_ARCH MATCHES "^powerpc*" OR
   TARGET_ARCH MATCHES "^ppc64*")
  add_definitions(-DPNG_POWERPC_VSX_OPT=0)
endif()

# Set definitions and sources for Intel.
if(TARGET_ARCH MATCHES "^i?86" OR
   TARGET_ARCH MATCHES "^x86_64*")
  add_definitions(-DPNG_INTEL_SSE_OPT=0)
endif()

# Set definitions and sources for MIPS.
if(TARGET_ARCH MATCHES "mipsel*" OR
   TARGET_ARCH MATCHES "mips64el*")
  add_definitions(-DPNG_MIPS_MSA_OPT=0)
endif()

endif(PNG_HARDWARE_OPTIMIZATIONS)

# Set PNG_LIB_NAME.
set(PNG_LIB_NAME png${PNGLIB_MAJOR}${PNGLIB_MINOR})

# Distinguish between debug and release builds.
set(CMAKE_DEBUG_POSTFIX "d")

include(CheckCSourceCompiles)
option(ld-version-script "Enable linker version script" ON)
if(ld-version-script AND NOT ANDROID AND NOT APPLE)
  # Check if LD supports linker scripts.
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/conftest.map" "
VERS_1 {
        global: sym;
        local: *;
};

VERS_2 {
        global: sym2;
                main;
} VERS_1;
")
  set(CMAKE_REQUIRED_FLAGS_SAVE ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} "-Wl,--version-script='${CMAKE_CURRENT_BINARY_DIR}/conftest.map'")
  check_c_source_compiles("void sym(void) {}
void sym2(void) {}
int main(void) {return 0;}
" HAVE_LD_VERSION_SCRIPT)
  if(NOT HAVE_LD_VERSION_SCRIPT)
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVE} "-Wl,-M -Wl,${CMAKE_CURRENT_BINARY_DIR}/conftest.map")
  check_c_source_compiles("void sym(void) {}
void sym2(void) {}
int main(void) {return 0;}
" HAVE_SOLARIS_LD_VERSION_SCRIPT)
  endif()
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVE})
  file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/conftest.map")
endif()

# Find symbol prefix.  Likely obsolete and unnecessary with recent
# toolchains (it's not done in many other projects).
function(symbol_prefix)
  set(SYMBOL_PREFIX)

  execute_process(COMMAND "${CMAKE_C_COMPILER}" "-E" "-"
                  INPUT_FILE /dev/null
                  OUTPUT_VARIABLE OUT
                  RESULT_VARIABLE STATUS)

  if(CPP_FAIL)
    message(WARNING "Failed to run the C preprocessor")
  endif()

  string(REPLACE "\n" ";" OUT "${OUT}")
  foreach(line ${OUT})
    string(REGEX MATCH "^PREFIX=" found_match "${line}")
    if(found_match)
      string(REGEX REPLACE "^PREFIX=(.*\)" "\\1" prefix "${line}")
      string(REGEX MATCH "__USER_LABEL_PREFIX__" found_match "${prefix}")
      if(found_match)
        string(REGEX REPLACE "(.*)__USER_LABEL_PREFIX__(.*)" "\\1\\2" prefix "${prefix}")
      endif()
      set(SYMBOL_PREFIX "${prefix}")
    endif()
  endforeach()

  message(STATUS "Symbol prefix: ${SYMBOL_PREFIX}")
  set(SYMBOL_PREFIX "${SYMBOL_PREFIX}" PARENT_SCOPE)
endfunction()

if(UNIX)
  symbol_prefix()
endif()

find_program(AWK NAMES gawk awk)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(NOT AWK OR ANDROID OR IOS)
  # No awk available to generate sources; use pre-built pnglibconf.h
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt
                 ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
  add_custom_target(genfiles) # Dummy
else()
  # Copy the awk scripts, converting their line endings to Unix (LF)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/checksym.awk
                 ${CMAKE_CURRENT_BINARY_DIR}/scripts/checksym.awk
                 @ONLY
                 NEWLINE_STYLE LF)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/options.awk
                 ${CMAKE_CURRENT_BINARY_DIR}/scripts/options.awk
                 @ONLY
                 NEWLINE_STYLE LF)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/dfn.awk
                 ${CMAKE_CURRENT_BINARY_DIR}/scripts/dfn.awk
                 @ONLY
                 NEWLINE_STYLE LF)

  # Generate .chk from .out with awk:
  # generate_chk(INPUT inputfile OUTPUT outputfile [DEPENDS dep1 [dep2...]])
  include(CMakeParseArguments)
  function(generate_chk)
    set(options)
    set(oneValueArgs INPUT OUTPUT)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(_GC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT _GC_INPUT)
      message(FATAL_ERROR "generate_chk: Missing INPUT argument")
    endif()
    if(NOT _GC_OUTPUT)
      message(FATAL_ERROR "generate_chk: Missing OUTPUT argument")
    endif()

    add_custom_command(OUTPUT "${_GC_OUTPUT}"
                       COMMAND "${CMAKE_COMMAND}"
                               "-DINPUT=${_GC_INPUT}"
                               "-DOUTPUT=${_GC_OUTPUT}"
                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/genchk.cmake"
                       DEPENDS "${_GC_INPUT}" ${_GC_DEPENDS}
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endfunction()

  # Generate .out from .c with awk
  # generate_out(INPUT inputfile OUTPUT outputfile [DEPENDS dep1 [dep2...]])
  function(generate_out)
    set(options)
    set(oneValueArgs INPUT OUTPUT)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(_GO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT _GO_INPUT)
      message(FATAL_ERROR "generate_out: Missing INPUT argument")
    endif()
    if(NOT _GO_OUTPUT)
      message(FATAL_ERROR "generate_out: Missing OUTPUT argument")
    endif()

    add_custom_command(OUTPUT "${_GO_OUTPUT}"
                       COMMAND "${CMAKE_COMMAND}"
                               "-DINPUT=${_GO_INPUT}"
                               "-DOUTPUT=${_GO_OUTPUT}"
                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/genout.cmake"
                       DEPENDS "${_GO_INPUT}" ${_GO_DEPENDS}
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endfunction()

  # Generate specific source file with awk
  # generate_source(OUTPUT outputfile [DEPENDS dep1 [dep2...]])
  function(generate_source)
    set(options)
    set(oneValueArgs OUTPUT)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(_GSO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT _GSO_OUTPUT)
      message(FATAL_ERROR "generate_source: Missing OUTPUT argument")
    endif()

    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_GSO_OUTPUT}"
                       COMMAND "${CMAKE_COMMAND}"
                               "-DOUTPUT=${_GSO_OUTPUT}"
                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake"
                       DEPENDS ${_GSO_DEPENDS}
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endfunction()

  # Copy file
  # generate_copy(INPUT inputfile OUTPUT outputfile [DEPENDS dep1 [dep2...]])
  function(generate_copy)
    set(options)
    set(oneValueArgs INPUT OUTPUT)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(_GCO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_custom_command(OUTPUT "${_GCO_OUTPUT}"
                       COMMAND "${CMAKE_COMMAND}"
                               -E remove "${_GCO_OUTPUT}"
                       COMMAND "${CMAKE_COMMAND}"
                               -E copy "${_GCO_INPUT}" "${_GCO_OUTPUT}"
                       DEPENDS "${source}" ${_GCO_DEPENDS})
  endfunction()

  # Generate scripts/pnglibconf.h
  generate_source(OUTPUT "scripts/pnglibconf.c"
                  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.dfa"
                          "${CMAKE_CURRENT_BINARY_DIR}/scripts/options.awk"
                          "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h")
  add_custom_target(scripts_pnglibconf_c DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/pnglibconf.c")

  # Generate pnglibconf.c
  generate_source(OUTPUT "pnglibconf.c"
                  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.dfa"
                          "${CMAKE_CURRENT_BINARY_DIR}/scripts/options.awk"
                          "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h")
  add_custom_target(pnglibconf_c DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.c")

  if(PNG_PREFIX)
    set(PNGLIBCONF_H_EXTRA_DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out"
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/macro.lst")
    set(PNGPREFIX_H_EXTRA_DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out")
  endif()

  generate_out(INPUT "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out"
               DEPENDS pnglibconf_c)
  add_custom_target(pnglibconf_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out")

  # Generate pnglibconf.h
  generate_source(OUTPUT "pnglibconf.h"
                  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out" pnglibconf_out
                          ${PNGLIBCONF_H_EXTRA_DEPENDS})
  add_custom_target(pnglibconf_h DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h")

  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/intprefix.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out"
               DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h" pnglibconf_h)
  add_custom_target(scripts_intprefix_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out")

  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/prefix.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out"
               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
                       "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out" pnglibconf_out)
  add_custom_target(scripts_prefix_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out")

  # Generate pngprefix.h
  generate_source(OUTPUT "pngprefix.h"
                  DEPENDS ${PNGPREFIX_H_EXTRA_DEPENDS})
  add_custom_target(pngprefix_h DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pngprefix.h")

  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sym.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out"
               DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h" pnglibconf_h)
  add_custom_target(scripts_sym_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out")

  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/symbols.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out"
               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt")
  add_custom_target(scripts_symbols_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out")

  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/vers.c"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out"
               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
                       "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h" pnglibconf_h)
  add_custom_target(scripts_vers_out DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out")

  generate_chk(INPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out"
               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk"
               DEPENDS scripts_symbols_out
                       "${CMAKE_CURRENT_BINARY_DIR}/scripts/checksym.awk"
                       "${CMAKE_CURRENT_SOURCE_DIR}/scripts/symbols.def")

  add_custom_target(scripts_symbols_chk
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk")

  generate_copy(INPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out"
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libpng.sym"
                DEPENDS scripts_sym_out)
  generate_copy(INPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out"
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libpng.vers"
                DEPENDS scripts_vers_out)

  add_custom_target(genvers
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libpng.vers")
  add_custom_target(gensym
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libpng.sym")

  add_custom_target("genprebuilt"
                    COMMAND "${CMAKE_COMMAND}"
                            "-DOUTPUT=scripts/pnglibconf.h.prebuilt"
                            -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

  # A single target handles generation of all generated files.
  add_custom_target(genfiles
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libpng.sym" gensym
                            "${CMAKE_CURRENT_BINARY_DIR}/libpng.vers" genvers
                            "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.c" pnglibconf_c
                            "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h" pnglibconf_h
                            "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out" pnglibconf_out
                            "${CMAKE_CURRENT_BINARY_DIR}/pngprefix.h" pngprefix_h
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out" scripts_intprefix_out
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/pnglibconf.c" scripts_pnglibconf_c
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out" scripts_prefix_out
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out" scripts_sym_out
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk" scripts_symbols_chk
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out" scripts_symbols_out
                            "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out" scripts_vers_out)
endif(NOT AWK OR ANDROID OR IOS)

# List the source code files.
set(libpng_public_hdrs
    png.h
    pngconf.h
    "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h"
)
set(libpng_private_hdrs
    pngpriv.h
    pngdebug.h
    pnginfo.h
    pngstruct.h
)
if(AWK AND NOT ANDROID AND NOT IOS)
  list(APPEND libpng_private_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pngprefix.h")
endif()
set(libpng_sources
    ${libpng_public_hdrs}
    ${libpng_private_hdrs}
    png.c
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtrans.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c
    ${libpng_arm_sources}
    ${libpng_intel_sources}
    ${libpng_mips_sources}
    ${libpng_powerpc_sources}
)
set(pngtest_sources
    pngtest.c
)
set(pngvalid_sources
    contrib/libtests/pngvalid.c
)
set(pngstest_sources
    contrib/libtests/pngstest.c
)
set(pngunknown_sources
    contrib/libtests/pngunknown.c
)
set(pngimage_sources
    contrib/libtests/pngimage.c
)
set(pngfix_sources
    contrib/tools/pngfix.c
)
set(png_fix_itxt_sources
    contrib/tools/png-fix-itxt.c
)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

if(PNG_DEBUG)
  add_definitions(-DPNG_DEBUG)
endif()

# Now build our target.
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ZLIB_INCLUDE_DIRS})

unset(PNG_LIB_TARGETS)

if(PNG_SHARED)
  add_library(png SHARED ${libpng_sources})
  set(PNG_LIB_TARGETS png)
  set_target_properties(png PROPERTIES OUTPUT_NAME ${PNG_LIB_NAME})
  add_dependencies(png genfiles)
  if(MSVC)
    # MVC does not append 'lib'. Do it here, to have consistent name.
    set_target_properties(png PROPERTIES PREFIX "lib")
    set_target_properties(png PROPERTIES IMPORT_PREFIX "lib")
  endif()
  target_link_libraries(png ${ZLIB_LIBRARIES} ${M_LIBRARY})

  if(UNIX AND AWK)
    if(HAVE_LD_VERSION_SCRIPT)
      set_target_properties(png PROPERTIES
                            LINK_FLAGS "-Wl,--version-script='${CMAKE_CURRENT_BINARY_DIR}/libpng.vers'")
    elseif(HAVE_SOLARIS_LD_VERSION_SCRIPT)
      set_target_properties(png PROPERTIES
                            LINK_FLAGS "-Wl,-M -Wl,'${CMAKE_CURRENT_BINARY_DIR}/libpng.vers'")
    endif()
  endif()
endif()

if(PNG_STATIC)
  # does not work without changing name
  set(PNG_LIB_NAME_STATIC png_static)
  add_library(png_static STATIC ${libpng_sources})
  add_dependencies(png_static genfiles)
  # MSVC doesn't use a different file extension for shared vs. static
  # libs. We are able to change OUTPUT_NAME to remove the _static
  # for all other platforms.
  if(NOT MSVC)
    set_target_properties(png_static PROPERTIES
                          OUTPUT_NAME "${PNG_LIB_NAME}"
                          CLEAN_DIRECT_OUTPUT 1)
  else()
    set_target_properties(png_static PROPERTIES
                          OUTPUT_NAME "${PNG_LIB_NAME}_static"
                          CLEAN_DIRECT_OUTPUT 1)
  endif()
  list(APPEND PNG_LIB_TARGETS png_static)
  if(MSVC)
    # MSVC does not append 'lib'. Do it here, to have consistent name.
    set_target_properties(png_static PROPERTIES PREFIX "lib")
  endif()
  target_link_libraries(png_static ${ZLIB_LIBRARIES} ${M_LIBRARY})
endif()

if(PNG_FRAMEWORK)
  set(PNG_LIB_NAME_FRAMEWORK png_framework)
  add_library(png_framework SHARED ${libpng_sources})
  add_dependencies(png_framework genfiles)
  list(APPEND PNG_LIB_TARGETS png_framework)
  set_target_properties(png_framework PROPERTIES
                        FRAMEWORK TRUE
                        FRAMEWORK_VERSION ${PNGLIB_VERSION}
                        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PNGLIB_MAJOR}.${PNGLIB_MINOR}
                        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PNGLIB_VERSION}
                        MACOSX_FRAMEWORK_IDENTIFIER org.libpng.libpng
                        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
                        PUBLIC_HEADER "${libpng_public_hdrs}"
                        OUTPUT_NAME png)
  target_link_libraries(png_framework ${ZLIB_LIBRARIES} ${M_LIBRARY})
endif()

if(NOT PNG_LIB_TARGETS)
  message(SEND_ERROR "No library variant selected to build. "
                     "Please enable at least one of the following options: "
                     "PNG_STATIC, PNG_SHARED, PNG_FRAMEWORK")
endif()

if(PNG_SHARED AND WIN32)
  set_target_properties(png PROPERTIES
                        DEFINE_SYMBOL PNG_BUILD_DLL)
endif()

function(png_add_test)
  set(options)
  set(oneValueArgs NAME COMMAND)
  set(multiValueArgs OPTIONS FILES)
  cmake_parse_arguments(_PAT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT _PAT_NAME)
    message(FATAL_ERROR "png_add_test: Missing NAME argument")
  endif()
  if(NOT _PAT_COMMAND)
    message(FATAL_ERROR "png_add_test: Missing COMMAND argument")
  endif()

  set(TEST_OPTIONS "${_PAT_OPTIONS}")
  set(TEST_FILES "${_PAT_FILES}")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scripts/test.cmake.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/tests/${_PAT_NAME}.cmake"
                 @ONLY)
  add_test(NAME "${_PAT_NAME}"
           COMMAND "${CMAKE_COMMAND}"
                   "-DLIBPNG=$<TARGET_FILE:png>"
                   "-DTEST_COMMAND=$<TARGET_FILE:${_PAT_COMMAND}>"
                   -P "${CMAKE_CURRENT_BINARY_DIR}/tests/${_PAT_NAME}.cmake")
endfunction()

if(PNG_TESTS AND PNG_SHARED)
  # Find test PNG files by globbing, but sort lists to ensure
  # consistency between different filesystems.
  file(GLOB PNGSUITE_PNGS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/pngsuite/*.png")
  list(SORT PNGSUITE_PNGS)
  file(GLOB TEST_PNGS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/testpngs/*.png")
  list(SORT TEST_PNGS)

  set(PNGTEST_PNG "${CMAKE_CURRENT_SOURCE_DIR}/pngtest.png")

  add_executable(pngtest ${pngtest_sources})
  target_link_libraries(pngtest png)

  png_add_test(NAME pngtest
               COMMAND pngtest
               FILES "${PNGTEST_PNG}")

  add_executable(pngvalid ${pngvalid_sources})
  target_link_libraries(pngvalid png)

  png_add_test(NAME pngvalid-gamma-16-to-8
               COMMAND pngvalid
               OPTIONS --gamma-16-to-8)
  png_add_test(NAME pngvalid-gamma-alpha-mode
               COMMAND pngvalid
               OPTIONS --gamma-alpha-mode)
  png_add_test(NAME pngvalid-gamma-background
               COMMAND pngvalid
               OPTIONS --gamma-background)
  png_add_test(NAME pngvalid-gamma-expand16-alpha-mode
               COMMAND pngvalid
               OPTIONS --gamma-alpha-mode --expand16)
  png_add_test(NAME pngvalid-gamma-expand16-background
               COMMAND pngvalid
               OPTIONS --gamma-background --expand16)
  png_add_test(NAME pngvalid-gamma-expand16-transform
               COMMAND pngvalid
               OPTIONS --gamma-transform --expand16)
  png_add_test(NAME pngvalid-gamma-sbit
               COMMAND pngvalid
               OPTIONS --gamma-sbit)
  png_add_test(NAME pngvalid-gamma-threshold
               COMMAND pngvalid
               OPTIONS --gamma-threshold)
  png_add_test(NAME pngvalid-gamma-transform
               COMMAND pngvalid
               OPTIONS --gamma-transform)
  png_add_test(NAME pngvalid-progressive-interlace-standard
               COMMAND pngvalid
               OPTIONS --standard --progressive-read --interlace)
  png_add_test(NAME pngvalid-progressive-size
               COMMAND pngvalid
               OPTIONS --size --progressive-read)
  png_add_test(NAME pngvalid-progressive-standard
               COMMAND pngvalid
               OPTIONS --standard --progressive-read)
  png_add_test(NAME pngvalid-standard
               COMMAND pngvalid
               OPTIONS --standard)
  png_add_test(NAME pngvalid-transform
               COMMAND pngvalid
               OPTIONS --transform)

  add_executable(pngstest ${pngstest_sources})
  target_link_libraries(pngstest png)

  foreach(gamma_type 1.8 linear none sRGB)
    foreach(alpha_type none alpha)
      set(PNGSTEST_FILES)
      foreach(test_png ${TEST_PNGS})
        string(REGEX MATCH ".*-linear[-.].*" TEST_PNG_LINEAR "${test_png}")
        string(REGEX MATCH ".*-sRGB[-.].*" TEST_PNG_SRGB "${test_png}")
        string(REGEX MATCH ".*-1.8[-.].*" TEST_PNG_G18 "${test_png}")
        string(REGEX MATCH ".*-alpha-.*" TEST_PNG_ALPHA "${test_png}")

        set(TEST_PNG_VALID TRUE)

        if(TEST_PNG_ALPHA)
          if(NOT "${alpha_type}" STREQUAL "alpha")
            set(TEST_PNG_VALID FALSE)
          endif()
        else()
          if("${alpha_type}" STREQUAL "alpha")
            set(TEST_PNG_VALID FALSE)
          endif()
        endif()

        if(TEST_PNG_LINEAR)
          if(NOT "${gamma_type}" STREQUAL "linear")
            set(TEST_PNG_VALID FALSE)
          endif()
        elseif(TEST_PNG_SRGB)
          if(NOT "${gamma_type}" STREQUAL "sRGB")
            set(TEST_PNG_VALID FALSE)
          endif()
        elseif(TEST_PNG_G18)
          if(NOT "${gamma_type}" STREQUAL "1.8")
            set(TEST_PNG_VALID FALSE)
          endif()
        else()
          if(NOT "${gamma_type}" STREQUAL "none")
            set(TEST_PNG_VALID FALSE)
          endif()
        endif()

        if(TEST_PNG_VALID)
          list(APPEND PNGSTEST_FILES "${test_png}")
        endif()
      endforeach()
      # Should already be sorted, but sort anyway to be certain.
      list(SORT PNGSTEST_FILES)
      png_add_test(NAME pngstest-${gamma_type}-${alpha_type}
                   COMMAND pngstest
                   OPTIONS --tmpfile "${gamma_type}-${alpha_type}-" --log
                   FILES ${PNGSTEST_FILES})
    endforeach()
  endforeach()

  add_executable(pngunknown ${pngunknown_sources})
  target_link_libraries(pngunknown png)

  png_add_test(NAME pngunknown-discard
               COMMAND pngunknown
               OPTIONS --strict default=discard
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-IDAT
               COMMAND pngunknown
               OPTIONS --strict default=discard IDAT=save
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-if-safe
               COMMAND pngunknown
               OPTIONS --strict default=if-safe
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-sAPI
               COMMAND pngunknown
               OPTIONS --strict bKGD=save cHRM=save gAMA=save all=discard iCCP=save sBIT=save sRGB=save
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-save
               COMMAND pngunknown
               OPTIONS --strict default=save
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-sTER
               COMMAND pngunknown
               OPTIONS --strict sTER=if-safe
               FILES "${PNGTEST_PNG}")
  png_add_test(NAME pngunknown-vpAg
               COMMAND pngunknown
               OPTIONS --strict vpAg=if-safe
               FILES "${PNGTEST_PNG}")

  add_executable(pngimage ${pngimage_sources})
  target_link_libraries(pngimage png)

  png_add_test(NAME pngimage-quick
               COMMAND pngimage
               OPTIONS --list-combos --log
               FILES ${PNGSUITE_PNGS})
  png_add_test(NAME pngimage-full
               COMMAND pngimage
               OPTIONS --exhaustive --list-combos --log
               FILES ${PNGSUITE_PNGS})
endif()

if(PNG_SHARED AND PNG_EXECUTABLES)
  add_executable(pngfix ${pngfix_sources})
  target_link_libraries(pngfix png)
  set(PNG_BIN_TARGETS pngfix)

  add_executable(png-fix-itxt ${png_fix_itxt_sources})
  target_link_libraries(png-fix-itxt ${ZLIB_LIBRARIES} ${M_LIBRARY})
  list(APPEND PNG_BIN_TARGETS png-fix-itxt)
endif()

# Creates a symlink from src to dest (if possible), or, alternatively,
# copies src to dest if different.
include(CMakeParseArguments)
function(create_symlink DEST_FILE)
  cmake_parse_arguments(S "" "FILE;TARGET" "" ${ARGN})

  if(NOT S_TARGET AND NOT S_FILE)
    message(FATAL_ERROR "create_symlink: Missing TARGET or FILE argument")
  endif()

  if(S_TARGET AND S_FILE)
    message(FATAL_ERROR "create_symlink: "
                        "Both source file ${S_FILE} and build target ${S_TARGET} arguments are present; "
                        "can only have one")
  endif()

  if(S_FILE)
    # If we don't need to symlink something that's coming from a build target,
    # we can go ahead and symlink/copy at configure time.
    if(CMAKE_HOST_WIN32 AND NOT CYGWIN)
      execute_process(COMMAND "${CMAKE_COMMAND}"
                              -E copy_if_different
                              ${S_FILE} ${DEST_FILE}
                      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    else()
      execute_process(COMMAND "${CMAKE_COMMAND}"
                              -E create_symlink
                              ${S_FILE} ${DEST_FILE}
                      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
  endif()

  if(S_TARGET)
    # We need to use generator expressions, which can be a bit tricky.
    # For simplicity, make the symlink a POST_BUILD step, and use the TARGET
    # signature of add_custom_command.
    if(CMAKE_HOST_WIN32 AND NOT CYGWIN)
      add_custom_command(TARGET ${S_TARGET}
                         POST_BUILD
                         COMMAND "${CMAKE_COMMAND}"
                                 -E copy_if_different
                                 $<TARGET_LINKER_FILE_NAME:${S_TARGET}>
                                 $<TARGET_LINKER_FILE_DIR:${S_TARGET}>/${DEST_FILE})
    else()
      add_custom_command(TARGET ${S_TARGET}
                         POST_BUILD
                         COMMAND "${CMAKE_COMMAND}"
                                 -E create_symlink
                                 $<TARGET_LINKER_FILE_NAME:${S_TARGET}>
                                 $<TARGET_LINKER_FILE_DIR:${S_TARGET}>/${DEST_FILE})
    endif()
  endif()
endfunction()

# Create source generation scripts.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/genchk.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/scripts/genchk.cmake
               @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/genout.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/scripts/genout.cmake
               @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/gensrc.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake
               @ONLY)

# libpng is a library so default to 'lib'
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()

# Create pkgconfig files.
# We use the same files like ./configure, so we have to set its vars.
# Only do this on Windows for Cygwin - the files don't make much sense
# outside of a UNIX look-alike.
if(NOT WIN32 OR CYGWIN OR MINGW)
  set(prefix      ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix ${CMAKE_INSTALL_PREFIX})
  set(libdir      ${CMAKE_INSTALL_FULL_LIBDIR})
  set(includedir  ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  set(LIBS        "-lz -lm")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng.pc.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${PNGLIB_NAME}.pc
                 @ONLY)
  create_symlink(libpng.pc FILE ${PNGLIB_NAME}.pc)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng-config.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${PNGLIB_NAME}-config
                 @ONLY)
  create_symlink(libpng-config FILE ${PNGLIB_NAME}-config)
endif()

# Set up links.
if(PNG_SHARED)
  set_target_properties(png PROPERTIES
    VERSION ${PNGLIB_SHARED_VERSION}
    SOVERSION ${PNGLIB_SHARED_SOVERSION}
    CLEAN_DIRECT_OUTPUT 1)
endif()

# Install.
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${PNG_LIB_TARGETS}
          EXPORT libpng
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

  if(PNG_SHARED)
    # Create a symlink for libpng.dll.a => libpng16.dll.a on Cygwin
    if(CYGWIN OR MINGW)
      create_symlink(libpng${CMAKE_IMPORT_LIBRARY_SUFFIX} TARGET png)
      install(FILES $<TARGET_LINKER_FILE_DIR:png>/libpng${CMAKE_IMPORT_LIBRARY_SUFFIX}
              DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()

    if(NOT WIN32)
      create_symlink(libpng${CMAKE_SHARED_LIBRARY_SUFFIX} TARGET png)
      install(FILES $<TARGET_LINKER_FILE_DIR:png>/libpng${CMAKE_SHARED_LIBRARY_SUFFIX}
              DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
  endif()

  if(PNG_STATIC)
    if(NOT WIN32 OR CYGWIN OR MINGW)
      create_symlink(libpng${CMAKE_STATIC_LIBRARY_SUFFIX} TARGET png_static)
      install(FILES $<TARGET_LINKER_FILE_DIR:png_static>/libpng${CMAKE_STATIC_LIBRARY_SUFFIX}
              DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
  endif()
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL)
  install(FILES ${libpng_public_hdrs}
          DESTINATION include)
  install(FILES ${libpng_public_hdrs}
          DESTINATION include/${PNGLIB_NAME})
endif()
if(NOT SKIP_INSTALL_EXECUTABLES AND NOT SKIP_INSTALL_ALL)
  if(NOT WIN32 OR CYGWIN OR MINGW)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libpng-config
            DESTINATION bin)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PNGLIB_NAME}-config
            DESTINATION bin)
  endif()
endif()

if(NOT SKIP_INSTALL_PROGRAMS AND NOT SKIP_INSTALL_ALL)
  install(TARGETS ${PNG_BIN_TARGETS}
          RUNTIME DESTINATION bin)
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL)
  # Install the man pages.
  install(FILES libpng.3 libpngpf.3
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)
  install(FILES png.5
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
  # Install the pkg-config files.
  if(NOT CMAKE_HOST_WIN32 OR CYGWIN OR MINGW)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libpng.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libpng-config
            DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PNGLIB_NAME}.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PNGLIB_NAME}-config
            DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()

# Create an export file that CMake users can include() to import our targets.
if(NOT SKIP_INSTALL_EXPORT AND NOT SKIP_INSTALL_ALL)
  install(EXPORT libpng
          DESTINATION lib/libpng
          FILE lib${PNG_LIB_NAME}.cmake)
endif()

# TODO: Create MSVC import lib for MinGW-compiled shared lib.
# pexports libpng.dll > libpng.def
# lib /def:libpng.def /machine:x86
