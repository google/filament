if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL source code and call cmake from there")
endif()

cmake_minimum_required(VERSION 2.8.11)
project(SDL2 C)

# !!! FIXME: this should probably do "MACOSX_RPATH ON" as a target property
# !!! FIXME:  for the SDL2 shared library (so you get an
# !!! FIXME:  install_name ("soname") of "@rpath/libSDL-whatever.dylib"
# !!! FIXME:  instead of "/usr/local/lib/libSDL-whatever.dylib"), but I'm
# !!! FIXME:  punting for now and leaving the existing behavior. Until this
# !!! FIXME:  properly resolved, this line silences a warning in CMake 3.0+.
# !!! FIXME:  remove it and this comment entirely once the problem is
# !!! FIXME:  properly resolved.
#cmake_policy(SET CMP0042 OLD)

include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCCompilerFlag)
include(CheckTypeSize)
include(CheckStructHasMember)
include(CMakeDependentOption)
include(FindPkgConfig)
include(GNUInstallDirs)
set(CMAKE_MODULE_PATH "${SDL2_SOURCE_DIR}/cmake")
include(${SDL2_SOURCE_DIR}/cmake/macros.cmake)
include(${SDL2_SOURCE_DIR}/cmake/sdlchecks.cmake)

# General settings
# Edit include/SDL_version.h and change the version, then:
#   SDL_MICRO_VERSION += 1;
#   SDL_INTERFACE_AGE += 1;
#   SDL_BINARY_AGE += 1;
# if any functions have been added, set SDL_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set SDL_BINARY_AGE and SDL_INTERFACE_AGE to 0.
set(SDL_MAJOR_VERSION 2)
set(SDL_MINOR_VERSION 0)
set(SDL_MICRO_VERSION 8)
set(SDL_INTERFACE_AGE 0)
set(SDL_BINARY_AGE 8)
set(SDL_VERSION "${SDL_MAJOR_VERSION}.${SDL_MINOR_VERSION}.${SDL_MICRO_VERSION}")

# Set defaults preventing destination file conflicts
set(SDL_CMAKE_DEBUG_POSTFIX "d"
    CACHE STRING "Name suffix for debug builds")

mark_as_advanced(CMAKE_IMPORT_LIBRARY_SUFFIX SDL_CMAKE_DEBUG_POSTFIX)

# Calculate a libtool-like version number
math(EXPR LT_CURRENT "${SDL_MICRO_VERSION} - ${SDL_INTERFACE_AGE}")
math(EXPR LT_AGE "${SDL_BINARY_AGE} - ${SDL_INTERFACE_AGE}")
math(EXPR LT_MAJOR "${LT_CURRENT}- ${LT_AGE}")
set(LT_REVISION "${SDL_INTERFACE_AGE}")
set(LT_RELEASE "${SDL_MAJOR_VERSION}.${SDL_MINOR_VERSION}")
set(LT_VERSION "${LT_MAJOR}.${LT_AGE}.${LT_REVISION}")

message(STATUS "${LT_VERSION} :: ${LT_AGE} :: ${LT_REVISION} :: ${LT_CURRENT} :: ${LT_RELEASE}")

# General settings & flags
set(LIBRARY_OUTPUT_DIRECTORY "build")
# Check for 64 or 32 bit
set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ARCH_64 TRUE)
  set(PROCESSOR_ARCH "x64")
else()
  set(ARCH_64 FALSE)
  set(PROCESSOR_ARCH "x86")
endif()
set(LIBNAME SDL2)
if(NOT LIBTYPE)
  set(LIBTYPE SHARED)
endif()

# Get the platform
if(WIN32)
  if(NOT WINDOWS)
    set(WINDOWS TRUE)
  endif()
elseif(UNIX AND NOT APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    set(LINUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kFreeBSD.*")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*")
    set(NETBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "kOpenBSD.*|OpenBSD.*")
    set(OPENBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*GNU.*")
    set(GNU TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*BSDI.*")
    set(BSDI TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "DragonFly.*|FreeBSD")
    set(FREEBSD TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "SYSV5.*")
    set(SYSV5 TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Solaris.*")
    set(SOLARIS TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
    set(HPUX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
    set(AIX TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES "Minix.*")
    set(MINIX TRUE)
  endif()
elseif(APPLE)
  if(CMAKE_SYSTEM_NAME MATCHES ".*Darwin.*")
    set(DARWIN TRUE)
  elseif(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
    set(MACOSX TRUE)
  endif()
  # TODO: iOS?
elseif(CMAKE_SYSTEM_NAME MATCHES "BeOS.*")
  message_error("BeOS support has been removed as of SDL 2.0.2.")
elseif(CMAKE_SYSTEM_NAME MATCHES "Haiku.*")
  set(HAIKU TRUE)
endif()

# Don't mistake osx for unix
if(UNIX AND NOT APPLE)
  set(UNIX_SYS ON)
else()
  set(UNIX_SYS OFF)
endif()

if(UNIX OR APPLE)
  set(UNIX_OR_MAC_SYS ON)
else()
  set(UNIX_OR_MAC_SYS OFF)
endif()

if (UNIX_OR_MAC_SYS AND NOT EMSCRIPTEN) # JavaScript does not yet have threading support, so disable pthreads when building for Emscripten.
  set(SDL_PTHREADS_ENABLED_BY_DEFAULT ON)
else()
  set(SDL_PTHREADS_ENABLED_BY_DEFAULT OFF)
endif()

# Default option knobs
if(APPLE OR ARCH_64)
  if(NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "arm")
    set(OPT_DEF_SSEMATH ON)
  endif()
endif()
if(UNIX OR MINGW OR MSYS)
  set(OPT_DEF_LIBC ON)
endif()

# Compiler info
if(CMAKE_COMPILER_IS_GNUCC)
  set(USE_GCC TRUE)
  set(OPT_DEF_ASM TRUE)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(USE_CLANG TRUE)
  set(OPT_DEF_ASM TRUE)
elseif(MSVC_VERSION GREATER 1400) # VisualStudio 8.0+
  set(OPT_DEF_ASM TRUE)
  #set(CMAKE_C_FLAGS "/ZI /WX- /
else()
  set(OPT_DEF_ASM FALSE)
endif()

if(USE_GCC OR USE_CLANG)
  set(OPT_DEF_GCC_ATOMICS ON)
endif()

# Default flags, if not set otherwise
if("$ENV{CFLAGS}" STREQUAL "")
  if(CMAKE_BUILD_TYPE STREQUAL "")
    if(USE_GCC OR USE_CLANG)
      set(CMAKE_C_FLAGS "-g -O3")
    endif()
  endif()
else()
  set(CMAKE_C_FLAGS "$ENV{CFLAGS}")
  list(APPEND EXTRA_CFLAGS "$ENV{CFLAGS}")
endif()
if(NOT ("$ENV{CFLAGS}" STREQUAL "")) # Hackish, but does the trick on Win32
  list(APPEND EXTRA_LDFLAGS "$ENV{LDFLAGS}")
endif()

if(MSVC)
  option(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" OFF)
  if(FORCE_STATIC_VCRT)
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()
    endforeach()
  endif()

  # Make sure /RTC1 is disabled, otherwise it will use functions from the CRT
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    string(REGEX REPLACE "/RTC(su|[1su])" "" ${flag_var} "${${flag_var}}")
  endforeach(flag_var)
endif()

# Those are used for pkg-config and friends, so that the SDL2.pc, sdl2-config,
# etc. are created correctly.
set(SDL_LIBS "-lSDL2")
set(SDL_CFLAGS "")

# Emscripten toolchain has a nonempty default value for this, and the checks 
# in this file need to change that, so remember the original value, and 
# restore back to that afterwards. For check_function_exists() to work in
# Emscripten, this value must be at its default value.
set(ORIG_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})

if(CYGWIN)
  # We build SDL on cygwin without the UNIX emulation layer
  include_directories("-I/usr/include/mingw")
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -mno-cygwin")
  check_c_source_compiles("int main(int argc, char **argv) {}"
    HAVE_GCC_NO_CYGWIN)
  set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
  if(HAVE_GCC_NO_CYGWIN)
    list(APPEND EXTRA_LDFLAGS "-mno-cygwin")
    list(APPEND SDL_LIBS "-mno-cygwin")
  endif()
  set(SDL_CFLAGS "${SDL_CFLAGS} -I/usr/include/mingw")
endif()

add_definitions(-DUSING_GENERATED_CONFIG_H)
# General includes
include_directories(${SDL2_BINARY_DIR}/include ${SDL2_SOURCE_DIR}/include)
if(USE_GCC OR USE_CLANG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -idirafter ${SDL2_SOURCE_DIR}/src/video/khronos")
else()
  include_directories(${SDL2_SOURCE_DIR}/src/video/khronos)
endif()

# All these ENABLED_BY_DEFAULT vars will default to ON if not specified, so
#  you only need to have a platform override them if they are disabling.
set(OPT_DEF_ASM TRUE)
if(EMSCRIPTEN)
  # Set up default values for the currently supported set of subsystems:
  # Emscripten/Javascript does not have assembly support, a dynamic library 
  # loading architecture, low-level CPU inspection or multithreading.
  set(OPT_DEF_ASM FALSE)
  set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)
  set(SDL_ATOMIC_ENABLED_BY_DEFAULT OFF)
  set(SDL_THREADS_ENABLED_BY_DEFAULT OFF)
  set(SDL_LOADSO_ENABLED_BY_DEFAULT OFF)
  set(SDL_CPUINFO_ENABLED_BY_DEFAULT OFF)
  set(SDL_DLOPEN_ENABLED_BY_DEFAULT OFF)
endif()

if (NOT DEFINED SDL_SHARED_ENABLED_BY_DEFAULT)
    set(SDL_SHARED_ENABLED_BY_DEFAULT ON)
endif()

set(SDL_SUBSYSTEMS
    Atomic Audio Video Render Events Joystick Haptic Power Threads Timers
    File Loadso CPUinfo Filesystem Dlopen)
foreach(_SUB ${SDL_SUBSYSTEMS})
  string(TOUPPER ${_SUB} _OPT)
  if (NOT DEFINED SDL_${_OPT}_ENABLED_BY_DEFAULT)
    set(SDL_${_OPT}_ENABLED_BY_DEFAULT ON)
  endif()
  option(SDL_${_OPT} "Enable the ${_SUB} subsystem" ${SDL_${_OPT}_ENABLED_BY_DEFAULT})
endforeach()

option_string(ASSERTIONS "Enable internal sanity checks (auto/disabled/release/enabled/paranoid)" "auto")
#set_option(DEPENDENCY_TRACKING "Use gcc -MMD -MT dependency tracking" ON)
set_option(LIBC                "Use the system C library" ${OPT_DEF_LIBC})
set_option(GCC_ATOMICS         "Use gcc builtin atomics" ${OPT_DEF_GCC_ATOMICS})
set_option(ASSEMBLY            "Enable assembly routines" ${OPT_DEF_ASM})
set_option(SSEMATH             "Allow GCC to use SSE floating point math" ${OPT_DEF_SSEMATH})
set_option(MMX                 "Use MMX assembly routines" ${OPT_DEF_ASM})
set_option(3DNOW               "Use 3Dnow! MMX assembly routines" ${OPT_DEF_ASM})
set_option(SSE                 "Use SSE assembly routines" ${OPT_DEF_ASM})
set_option(SSE2                "Use SSE2 assembly routines" ${OPT_DEF_SSEMATH})
set_option(SSE3                "Use SSE3 assembly routines" ${OPT_DEF_SSEMATH})
set_option(ALTIVEC             "Use Altivec assembly routines" ${OPT_DEF_ASM})
set_option(DISKAUDIO           "Support the disk writer audio driver" ON)
set_option(DUMMYAUDIO          "Support the dummy audio driver" ON)
set_option(VIDEO_DIRECTFB      "Use DirectFB video driver" OFF)
dep_option(DIRECTFB_SHARED     "Dynamically load directfb support" ON "VIDEO_DIRECTFB" OFF)
set_option(VIDEO_DUMMY         "Use dummy video driver" ON)
set_option(VIDEO_OPENGL        "Include OpenGL support" ON)
set_option(VIDEO_OPENGLES      "Include OpenGL ES support" ON)
set_option(PTHREADS            "Use POSIX threads for multi-threading" ${SDL_PTHREADS_ENABLED_BY_DEFAULT})
dep_option(PTHREADS_SEM        "Use pthread semaphores" ON "PTHREADS" OFF)
set_option(SDL_DLOPEN          "Use dlopen for shared object loading" ${SDL_DLOPEN_ENABLED_BY_DEFAULT})
set_option(OSS                 "Support the OSS audio API" ${UNIX_SYS})
set_option(ALSA                "Support the ALSA audio API" ${UNIX_SYS})
dep_option(ALSA_SHARED         "Dynamically load ALSA audio support" ON "ALSA" OFF)
set_option(JACK                "Support the JACK audio API" ${UNIX_SYS})
dep_option(JACK_SHARED         "Dynamically load JACK audio support" ON "JACK" OFF)
set_option(ESD                 "Support the Enlightened Sound Daemon" ${UNIX_SYS})
dep_option(ESD_SHARED          "Dynamically load ESD audio support" ON "ESD" OFF)
set_option(PULSEAUDIO          "Use PulseAudio" ${UNIX_SYS})
dep_option(PULSEAUDIO_SHARED   "Dynamically load PulseAudio support" ON "PULSEAUDIO" OFF)
set_option(ARTS                "Support the Analog Real Time Synthesizer" ${UNIX_SYS})
dep_option(ARTS_SHARED         "Dynamically load aRts audio support" ON "ARTS" OFF)
set_option(NAS                 "Support the NAS audio API" ${UNIX_SYS})
set_option(NAS_SHARED          "Dynamically load NAS audio API" ${UNIX_SYS})
set_option(SNDIO               "Support the sndio audio API" ${UNIX_SYS})
set_option(FUSIONSOUND         "Use FusionSound audio driver" OFF)
dep_option(FUSIONSOUND_SHARED  "Dynamically load fusionsound audio support" ON "FUSIONSOUND" OFF)
set_option(LIBSAMPLERATE       "Use libsamplerate for audio rate conversion" ${UNIX_SYS})
dep_option(LIBSAMPLERATE_SHARED "Dynamically load libsamplerate" ON "LIBSAMPLERATE" OFF)
set_option(RPATH               "Use an rpath when linking SDL" ${UNIX_SYS})
set_option(CLOCK_GETTIME       "Use clock_gettime() instead of gettimeofday()" OFF)
set_option(INPUT_TSLIB         "Use the Touchscreen library for input" ${UNIX_SYS})
set_option(VIDEO_X11           "Use X11 video driver" ${UNIX_SYS})
set_option(VIDEO_WAYLAND       "Use Wayland video driver" ${UNIX_SYS})
dep_option(WAYLAND_SHARED      "Dynamically load Wayland support" ON "VIDEO_WAYLAND" OFF)
dep_option(VIDEO_WAYLAND_QT_TOUCH  "QtWayland server support for Wayland video driver" ON "VIDEO_WAYLAND" OFF)
set_option(VIDEO_MIR           "Use Mir video driver" ${UNIX_SYS})
dep_option(MIR_SHARED          "Dynamically load Mir support" ON "VIDEO_MIR" OFF)
set_option(VIDEO_RPI           "Use Raspberry Pi video driver" ${UNIX_SYS})
dep_option(X11_SHARED          "Dynamically load X11 support" ON "VIDEO_X11" OFF)
set(SDL_X11_OPTIONS Xcursor Xinerama XInput Xrandr Xscrnsaver XShape Xvm)
foreach(_SUB ${SDL_X11_OPTIONS})
  string(TOUPPER "VIDEO_X11_${_SUB}" _OPT)
  dep_option(${_OPT}           "Enable ${_SUB} support" ON "VIDEO_X11" OFF)
endforeach()
set_option(VIDEO_COCOA         "Use Cocoa video driver" ${APPLE})
set_option(DIRECTX             "Use DirectX for Windows audio/video" ${WINDOWS})
set_option(RENDER_D3D          "Enable the Direct3D render driver" ${WINDOWS})
set_option(VIDEO_VIVANTE       "Use Vivante EGL video driver" ${UNIX_SYS})
dep_option(VIDEO_VULKAN        "Enable Vulkan support" ON "ANDROID OR APPLE OR LINUX OR WINDOWS" OFF)
set_option(VIDEO_KMSDRM        "Use KMS DRM video driver" ${UNIX_SYS})
dep_option(KMSDRM_SHARED       "Dynamically load KMS DRM support" ON "VIDEO_KMSDRM" OFF)

# TODO: We should (should we?) respect cmake's ${BUILD_SHARED_LIBS} flag here
# The options below are for compatibility to configure's default behaviour.
set(SDL_SHARED ${SDL_SHARED_ENABLED_BY_DEFAULT} CACHE BOOL "Build a shared version of the library")
set(SDL_STATIC ON CACHE BOOL "Build a static version of the library")

dep_option(SDL_STATIC_PIC      "Static version of the library should be built with Position Independent Code" OFF "SDL_STATIC" OFF)
set_option(SDL_TEST            "Build the test directory" OFF)

# General source files
file(GLOB SOURCE_FILES
  ${SDL2_SOURCE_DIR}/src/*.c
  ${SDL2_SOURCE_DIR}/src/atomic/*.c
  ${SDL2_SOURCE_DIR}/src/audio/*.c
  ${SDL2_SOURCE_DIR}/src/cpuinfo/*.c
  ${SDL2_SOURCE_DIR}/src/dynapi/*.c
  ${SDL2_SOURCE_DIR}/src/events/*.c
  ${SDL2_SOURCE_DIR}/src/file/*.c
  ${SDL2_SOURCE_DIR}/src/libm/*.c
  ${SDL2_SOURCE_DIR}/src/render/*.c
  ${SDL2_SOURCE_DIR}/src/render/*/*.c
  ${SDL2_SOURCE_DIR}/src/stdlib/*.c
  ${SDL2_SOURCE_DIR}/src/thread/*.c
  ${SDL2_SOURCE_DIR}/src/timer/*.c
  ${SDL2_SOURCE_DIR}/src/video/*.c
  ${SDL2_SOURCE_DIR}/src/video/yuv2rgb/*.c)


if(ASSERTIONS STREQUAL "auto")
  # Do nada - use optimization settings to determine the assertion level
elseif(ASSERTIONS STREQUAL "disabled")
  set(SDL_DEFAULT_ASSERT_LEVEL 0)
elseif(ASSERTIONS STREQUAL "release")
  set(SDL_DEFAULT_ASSERT_LEVEL 1)
elseif(ASSERTIONS STREQUAL "enabled")
  set(SDL_DEFAULT_ASSERT_LEVEL 2)
elseif(ASSERTIONS STREQUAL "paranoid")
  set(SDL_DEFAULT_ASSERT_LEVEL 3)
else()
  message_error("unknown assertion level")
endif()
set(HAVE_ASSERTIONS ${ASSERTIONS})

# Compiler option evaluation
if(USE_GCC OR USE_CLANG)
  # Check for -Wall first, so later things can override pieces of it.
  check_c_compiler_flag(-Wall HAVE_GCC_WALL)
  if(HAVE_GCC_WALL)
    list(APPEND EXTRA_CFLAGS "-Wall")
    if(HAIKU)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-multichar")
    endif()
  endif()

  check_c_compiler_flag(-Wdeclaration-after-statement HAVE_GCC_WDECLARATION_AFTER_STATEMENT)
  if(HAVE_GCC_WDECLARATION_AFTER_STATEMENT)
    check_c_compiler_flag(-Werror=declaration-after-statement HAVE_GCC_WERROR_DECLARATION_AFTER_STATEMENT)
    if(HAVE_GCC_WERROR_DECLARATION_AFTER_STATEMENT)
      list(APPEND EXTRA_CFLAGS "-Werror=declaration-after-statement")
    endif()
    list(APPEND EXTRA_CFLAGS "-Wdeclaration-after-statement")
  endif()

  if(DEPENDENCY_TRACKING)
    check_c_source_compiles("
        #if !defined(__GNUC__) || __GNUC__ < 3
        #error Dependency tracking requires GCC 3.0 or newer
        #endif
        int main(int argc, char **argv) { }" HAVE_DEPENDENCY_TRACKING)
  endif()

  if(GCC_ATOMICS)
    check_c_source_compiles("int main(int argc, char **argv) {
        int a;
        void *x, *y, *z;
        __sync_lock_test_and_set(&a, 4);
        __sync_lock_test_and_set(&x, y);
        __sync_fetch_and_add(&a, 1);
        __sync_bool_compare_and_swap(&a, 5, 10);
        __sync_bool_compare_and_swap(&x, y, z); }" HAVE_GCC_ATOMICS)
    if(NOT HAVE_GCC_ATOMICS)
      check_c_source_compiles("int main(int argc, char **argv) {
          int a;
          __sync_lock_test_and_set(&a, 1);
          __sync_lock_release(&a); }" HAVE_GCC_SYNC_LOCK_TEST_AND_SET)
    endif()
  endif()

  set(CMAKE_REQUIRED_FLAGS "-mpreferred-stack-boundary=2")
  check_c_source_compiles("int x = 0; int main(int argc, char **argv) {}"
    HAVE_GCC_PREFERRED_STACK_BOUNDARY)
  set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})

  set(CMAKE_REQUIRED_FLAGS "-fvisibility=hidden -Werror")
  check_c_source_compiles("
      #if !defined(__GNUC__) || __GNUC__ < 4
      #error SDL only uses visibility attributes in GCC 4 or newer
      #endif
      int main(int argc, char **argv) {}" HAVE_GCC_FVISIBILITY)
  if(HAVE_GCC_FVISIBILITY)
    list(APPEND EXTRA_CFLAGS "-fvisibility=hidden")
  endif()
  set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})

  check_c_compiler_flag(-Wshadow HAVE_GCC_WSHADOW)
  if(HAVE_GCC_WSHADOW)
    list(APPEND EXTRA_CFLAGS "-Wshadow")
  endif()

  if(APPLE)
    list(APPEND EXTRA_LDFLAGS "-Wl,-undefined,error")
  else()
    set(CMAKE_REQUIRED_FLAGS "-Wl,--no-undefined")
    check_c_compiler_flag("" HAVE_NO_UNDEFINED)
    set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    if(HAVE_NO_UNDEFINED)
      list(APPEND EXTRA_LDFLAGS "-Wl,--no-undefined")
    endif()
  endif()
endif()

if(ASSEMBLY)
  if(USE_GCC OR USE_CLANG)
    set(SDL_ASSEMBLY_ROUTINES 1)
    # TODO: Those all seem to be quite GCC specific - needs to be
    # reworked for better compiler support
    set(HAVE_ASSEMBLY TRUE)
    if(MMX)
      set(CMAKE_REQUIRED_FLAGS "-mmmx")
      check_c_source_compiles("
          #ifdef __MINGW32__
          #include <_mingw.h>
          #ifdef __MINGW64_VERSION_MAJOR
          #include <intrin.h>
          #else
          #include <mmintrin.h>
          #endif
          #else
          #include <mmintrin.h>
          #endif
          #ifndef __MMX__
          #error Assembler CPP flag not enabled
          #endif
          int main(int argc, char **argv) { }" HAVE_MMX)
      if(HAVE_MMX)
        list(APPEND EXTRA_CFLAGS "-mmmx")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    endif()

    if(3DNOW)
      set(CMAKE_REQUIRED_FLAGS "-m3dnow")
      check_c_source_compiles("
          #include <mm3dnow.h>
          #ifndef __3dNOW__
          #error Assembler CPP flag not enabled
          #endif
          int main(int argc, char **argv) {
            void *p = 0;
            _m_prefetch(p);
          }" HAVE_3DNOW)
      if(HAVE_3DNOW)
        list(APPEND EXTRA_CFLAGS "-m3dnow")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    endif()

    if(SSE)
      set(CMAKE_REQUIRED_FLAGS "-msse")
      check_c_source_compiles("
          #ifdef __MINGW32__
          #include <_mingw.h>
          #ifdef __MINGW64_VERSION_MAJOR
          #include <intrin.h>
          #else
          #include <xmmintrin.h>
          #endif
          #else
          #include <xmmintrin.h>
          #endif
          #ifndef __SSE__
          #error Assembler CPP flag not enabled
          #endif
          int main(int argc, char **argv) { }" HAVE_SSE)
      if(HAVE_SSE)
        list(APPEND EXTRA_CFLAGS "-msse")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    endif()

    if(SSE2)
      set(CMAKE_REQUIRED_FLAGS "-msse2")
      check_c_source_compiles("
          #ifdef __MINGW32__
          #include <_mingw.h>
          #ifdef __MINGW64_VERSION_MAJOR
          #include <intrin.h>
          #else
          #include <emmintrin.h>
          #endif
          #else
          #include <emmintrin.h>
          #endif
          #ifndef __SSE2__
          #error Assembler CPP flag not enabled
          #endif
          int main(int argc, char **argv) { }" HAVE_SSE2)
      if(HAVE_SSE2)
        list(APPEND EXTRA_CFLAGS "-msse2")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    endif()

    if(SSE3)
      set(CMAKE_REQUIRED_FLAGS "-msse3")
      check_c_source_compiles("
          #ifdef __MINGW32__
          #include <_mingw.h>
          #ifdef __MINGW64_VERSION_MAJOR
          #include <intrin.h>
          #else
          #include <pmmintrin.h>
          #endif
          #else
          #include <pmmintrin.h>
          #endif
          #ifndef __SSE3__
          #error Assembler CPP flag not enabled
          #endif
          int main(int argc, char **argv) { }" HAVE_SSE3)
      if(HAVE_SSE3)
        list(APPEND EXTRA_CFLAGS "-msse3")
      endif()
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
    endif()

    if(NOT SSEMATH)
      if(SSE OR SSE2 OR SSE3)
        if(USE_GCC)
          check_c_compiler_flag(-mfpmath=387 HAVE_FP_387)
          if(HAVE_FP_387)
            list(APPEND EXTRA_CFLAGS "-mfpmath=387")
          endif()
        endif()
        set(HAVE_SSEMATH TRUE)
      endif()
    endif()

    check_include_file("immintrin.h" HAVE_IMMINTRIN_H)

    if(ALTIVEC)
      set(CMAKE_REQUIRED_FLAGS "-maltivec")
      check_c_source_compiles("
          #include <altivec.h>
          vector unsigned int vzero() {
              return vec_splat_u32(0);
          }
          int main(int argc, char **argv) { }" HAVE_ALTIVEC_H_HDR)
      check_c_source_compiles("
          vector unsigned int vzero() {
              return vec_splat_u32(0);
          }
          int main(int argc, char **argv) { }" HAVE_ALTIVEC)
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
      if(HAVE_ALTIVEC OR HAVE_ALTIVEC_H_HDR)
        set(HAVE_ALTIVEC TRUE) # if only HAVE_ALTIVEC_H_HDR is set
        list(APPEND EXTRA_CFLAGS "-maltivec")
        set(SDL_ALTIVEC_BLITTERS 1)
        if(HAVE_ALTIVEC_H_HDR)
          set(HAVE_ALTIVEC_H 1)
        endif()
      endif()
    endif()
  elseif(MSVC_VERSION GREATER 1500)
    # TODO: SDL_cpuinfo.h needs to support the user's configuration wish
    # for MSVC - right now it is always activated
    if(NOT ARCH_64)
      set(HAVE_MMX TRUE)
      set(HAVE_3DNOW TRUE)
    endif()
    set(HAVE_SSE TRUE)
    set(HAVE_SSE2 TRUE)
    set(HAVE_SSE3 TRUE)
    set(SDL_ASSEMBLY_ROUTINES 1)
  endif()
# TODO:
#else()
#  if(USE_GCC OR USE_CLANG)
#    list(APPEND EXTRA_CFLAGS "-mno-sse" "-mno-sse2" "-mno-sse3" "-mno-mmx")
#  endif()
endif()

# TODO: Can't deactivate on FreeBSD? w/o LIBC, SDL_stdinc.h can't define
# anything.
if(LIBC)
  if(WINDOWS AND NOT MINGW)
    set(HAVE_LIBC TRUE)
    foreach(_HEADER stdio.h string.h wchar.h ctype.h math.h limits.h)
      string(TOUPPER "HAVE_${_HEADER}" _UPPER)
      string(REPLACE "." "_" _HAVE_H ${_UPPER})
      set(${_HAVE_H} 1)
    endforeach()
    set(HAVE_SIGNAL_H 1)
    foreach(_FN
            malloc calloc realloc free qsort abs memset memcpy memmove memcmp
            wcslen wcscmp
            strlen _strrev _strupr _strlwr strchr strrchr strstr itoa _ltoa
            _ultoa strtol strtoul strtoll strtod atoi atof strcmp strncmp
            _stricmp _strnicmp sscanf
            acos acosf asin asinf atan atanf atan2 atan2f ceil ceilf
            copysign copysignf cos cosf fabs fabsf floor floorf fmod fmodf
            log logf log10 log10f pow powf scalbn scalbnf sin sinf sqrt sqrtf tan tanf)
      string(TOUPPER ${_FN} _UPPER)
      set(HAVE_${_UPPER} 1)
    endforeach()
    if(NOT CYGWIN AND NOT MINGW)
      set(HAVE_ALLOCA 1)
    endif()
    set(HAVE_M_PI 1)
    add_definitions(-D_USE_MATH_DEFINES) # needed for M_PI
    set(STDC_HEADERS 1)
  else()
    set(HAVE_LIBC TRUE)
    check_include_file(sys/types.h HAVE_SYS_TYPES_H)
    foreach(_HEADER
            stdio.h stdlib.h stddef.h stdarg.h malloc.h memory.h string.h limits.h
            strings.h wchar.h inttypes.h stdint.h ctype.h math.h iconv.h signal.h libunwind.h)
      string(TOUPPER "HAVE_${_HEADER}" _UPPER)
      string(REPLACE "." "_" _HAVE_H ${_UPPER})
      check_include_file("${_HEADER}" ${_HAVE_H})
    endforeach()

    check_include_files("dlfcn.h;stdint.h;stddef.h;inttypes.h;stdlib.h;strings.h;string.h;float.h" STDC_HEADERS)
    check_type_size("size_t" SIZEOF_SIZE_T)
    check_symbol_exists(M_PI math.h HAVE_M_PI)
    # TODO: refine the mprotect check
    check_c_source_compiles("#include <sys/types.h>
                             #include <sys/mman.h>
                             int main() { }" HAVE_MPROTECT)
    foreach(_FN
            strtod malloc calloc realloc free getenv setenv putenv unsetenv
            qsort abs bcopy memset memcpy memmove memcmp strlen strlcpy strlcat
            _strrev _strupr _strlwr strchr strrchr strstr itoa _ltoa
            _uitoa _ultoa strtol strtoul _i64toa _ui64toa strtoll strtoull
            atoi atof strcmp strncmp _stricmp strcasecmp _strnicmp strncasecmp
            vsscanf vsnprintf fopen64 fseeko fseeko64 sigaction setjmp
            nanosleep sysconf sysctlbyname getauxval poll
            )
      string(TOUPPER ${_FN} _UPPER)
      set(_HAVEVAR "HAVE_${_UPPER}")
      check_function_exists("${_FN}" ${_HAVEVAR})
    endforeach()

    check_library_exists(m pow "" HAVE_LIBM)
    if(HAVE_LIBM)
      set(CMAKE_REQUIRED_LIBRARIES m)
      foreach(_FN
              atan atan2 ceil copysign cos cosf fabs floor log pow scalbn sin
              sinf sqrt sqrtf tan tanf acos asin)
        string(TOUPPER ${_FN} _UPPER)
        set(_HAVEVAR "HAVE_${_UPPER}")
        check_function_exists("${_FN}" ${_HAVEVAR})
      endforeach()
      set(CMAKE_REQUIRED_LIBRARIES)
      list(APPEND EXTRA_LIBS m)
    endif()

    check_library_exists(iconv iconv_open "" HAVE_LIBICONV)
    if(HAVE_LIBICONV)
      list(APPEND EXTRA_LIBS iconv)
      set(HAVE_ICONV 1)
    endif()

    if(NOT APPLE)
      check_include_file(alloca.h HAVE_ALLOCA_H)
      check_function_exists(alloca HAVE_ALLOCA)
    else()
      set(HAVE_ALLOCA_H 1)
      set(HAVE_ALLOCA 1)
    endif()

    check_struct_has_member("struct sigaction" "sa_sigaction" "signal.h" HAVE_SA_SIGACTION)
  endif()
else()
  if(WINDOWS)
    set(HAVE_STDARG_H 1)
    set(HAVE_STDDEF_H 1)
  endif()
endif()


# Enable/disable various subsystems of the SDL library
foreach(_SUB ${SDL_SUBSYSTEMS})
  string(TOUPPER ${_SUB} _OPT)
  if(NOT SDL_${_OPT})
    set(SDL_${_OPT}_DISABLED 1)
  endif()
endforeach()
if(SDL_JOYSTICK)
  file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${JOYSTICK_SOURCES})
endif()
if(SDL_HAPTIC)
  if(NOT SDL_JOYSTICK)
    # Haptic requires some private functions from the joystick subsystem.
    message_error("SDL_HAPTIC requires SDL_JOYSTICK, which is not enabled")
  endif()
  file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${HAPTIC_SOURCES})
endif()
if(SDL_POWER)
  file(GLOB POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${POWER_SOURCES})
endif()
# TODO: in configure.in, the test for LOADSO and SDL_DLOPEN is a bit weird:
# if LOADSO is not wanted, SDL_LOADSO_DISABLED is set
# If however on Unix or APPLE dlopen() is detected via CheckDLOPEN(),
# SDL_LOADSO_DISABLED will not be set, regardless of the LOADSO settings

# General SDL subsystem options, valid for all platforms
if(SDL_AUDIO)
  # CheckDummyAudio/CheckDiskAudio - valid for all platforms
  if(DUMMYAUDIO)
    set(SDL_AUDIO_DRIVER_DUMMY 1)
    file(GLOB DUMMYAUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/dummy/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${DUMMYAUDIO_SOURCES})
    set(HAVE_DUMMYAUDIO TRUE)
  endif()
  if(DISKAUDIO)
    set(SDL_AUDIO_DRIVER_DISK 1)
    file(GLOB DISKAUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/disk/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${DISKAUDIO_SOURCES})
    set(HAVE_DISKAUDIO TRUE)
  endif()
endif()

if(SDL_DLOPEN)
  # Relevant for Unix/Darwin only
  if(UNIX OR APPLE)
    CheckDLOPEN()
  endif()
endif()

if(SDL_VIDEO)
  if(VIDEO_DUMMY)
    set(SDL_VIDEO_DRIVER_DUMMY 1)
    file(GLOB VIDEO_DUMMY_SOURCES ${SDL2_SOURCE_DIR}/src/video/dummy/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${VIDEO_DUMMY_SOURCES})
    set(HAVE_VIDEO_DUMMY TRUE)
    set(HAVE_SDL_VIDEO TRUE)
  endif()
endif()

if(ANDROID)
  file(GLOB ANDROID_CORE_SOURCES ${SDL2_SOURCE_DIR}/src/core/android/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_CORE_SOURCES})

  # SDL_spinlock.c Needs to be compiled in ARM mode.
  # There seems to be no better way currently to set the ARM mode.
  # see: https://issuetracker.google.com/issues/62264618
  # Another option would be to set ARM mode to all compiled files
  check_c_compiler_flag(-marm HAVE_ARM_MODE)
  if(HAVE_ARM_MODE)
    set_source_files_properties(${SDL2_SOURCE_DIR}/src/atomic/SDL_spinlock.c PROPERTIES COMPILE_FLAGS -marm)
  endif()

  file(GLOB ANDROID_MAIN_SOURCES ${SDL2_SOURCE_DIR}/src/main/android/*.c)
  set(SDLMAIN_SOURCES ${SDLMAIN_SOURCES} ${ANDROID_MAIN_SOURCES})

  if(SDL_AUDIO)
    set(SDL_AUDIO_DRIVER_ANDROID 1)
    file(GLOB ANDROID_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/android/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_AUDIO_SOURCES})
    set(HAVE_SDL_AUDIO TRUE)
  endif()
  if(SDL_FILESYSTEM)
    set(SDL_FILESYSTEM_ANDROID 1)
    file(GLOB ANDROID_FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/android/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)
  endif()
  if(SDL_HAPTIC)
    set(SDL_HAPTIC_ANDROID 1)
    file(GLOB ANDROID_HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/android/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_HAPTIC_SOURCES})
    set(HAVE_SDL_HAPTIC TRUE)
  endif()
  if(SDL_JOYSTICK)
    set(SDL_JOYSTICK_ANDROID 1)
    file(GLOB ANDROID_JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/android/*.c ${SDL2_SOURCE_DIR}/src/joystick/steam/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_JOYSTICK_SOURCES})
    set(HAVE_SDL_JOYSTICK TRUE)
  endif()
  if(SDL_LOADSO)
    set(SDL_LOADSO_DLOPEN 1)
    file(GLOB LOADSO_SOURCES ${SDL2_SOURCE_DIR}/src/loadso/dlopen/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${LOADSO_SOURCES})
    set(HAVE_SDL_LOADSO TRUE)
  endif()
  if(SDL_POWER)
    set(SDL_POWER_ANDROID 1)
    file(GLOB ANDROID_POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/android/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_POWER_SOURCES})
    set(HAVE_SDL_POWER TRUE)
  endif()
  if(SDL_TIMERS)
    set(SDL_TIMER_UNIX 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)
  endif()
  if(SDL_VIDEO)
    set(SDL_VIDEO_DRIVER_ANDROID 1)
    file(GLOB ANDROID_VIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/android/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${ANDROID_VIDEO_SOURCES})
    set(HAVE_SDL_VIDEO TRUE)

    # Core stuff
    find_library(ANDROID_DL_LIBRARY dl)
    find_library(ANDROID_LOG_LIBRARY log)
    find_library(ANDROID_LIBRARY_LIBRARY android)
    list(APPEND EXTRA_LIBS ${ANDROID_DL_LIBRARY} ${ANDROID_LOG_LIBRARY} ${ANDROID_LIBRARY_LIBRARY})
    add_definitions(-DGL_GLEXT_PROTOTYPES)

    #enable gles
    if(VIDEO_OPENGLES)
      set(SDL_VIDEO_OPENGL_EGL 1)
      set(HAVE_VIDEO_OPENGLES TRUE)
      set(SDL_VIDEO_OPENGL_ES2 1)
      set(SDL_VIDEO_RENDER_OGL_ES2 1)

      find_library(OpenGLES1_LIBRARY GLESv1_CM)
      find_library(OpenGLES2_LIBRARY GLESv2)
      list(APPEND EXTRA_LIBS ${OpenGLES1_LIBRARY} ${OpenGLES2_LIBRARY})
    endif()

    CHECK_C_SOURCE_COMPILES("
    #if defined(__ARM_ARCH) && __ARM_ARCH < 7
    #error Vulkan doesn't work on this configuration
    #endif
    int main()
    {
        return 0;
    }
    " VULKAN_PASSED_ANDROID_CHECKS)
    if(NOT VULKAN_PASSED_ANDROID_CHECKS)
      set(VIDEO_VULKAN OFF)
      message(STATUS "Vulkan doesn't work on this configuration")
    endif()
  endif()

  CheckPTHREAD()

endif()

# Platform-specific options and settings
if(EMSCRIPTEN)
  # Hide noisy warnings that intend to aid mostly during initial stages of porting a new
  # project. Uncomment at will for verbose cross-compiling -I/../ path info.
  add_definitions(-Wno-warn-absolute-paths)
  if(SDL_AUDIO)
    set(SDL_AUDIO_DRIVER_EMSCRIPTEN 1)
    file(GLOB EM_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/emscripten/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${EM_AUDIO_SOURCES})
    set(HAVE_SDL_AUDIO TRUE)
  endif()
  if(SDL_FILESYSTEM)
    set(SDL_FILESYSTEM_EMSCRIPTEN 1)
    file(GLOB EM_FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/emscripten/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${EM_FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)
  endif()
  if(SDL_JOYSTICK)
    set(SDL_JOYSTICK_EMSCRIPTEN 1)
    file(GLOB EM_JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/emscripten/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${EM_JOYSTICK_SOURCES})
    set(HAVE_SDL_JOYSTICK TRUE)
  endif()
  if(SDL_POWER)
    set(SDL_POWER_EMSCRIPTEN 1)
    file(GLOB EM_POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/emscripten/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${EM_POWER_SOURCES})
    set(HAVE_SDL_POWER TRUE)
  endif()
  if(SDL_TIMERS)
    set(SDL_TIMER_UNIX 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)

    if(CLOCK_GETTIME)
      set(HAVE_CLOCK_GETTIME 1)
    endif()
  endif()
  if(SDL_VIDEO)
    set(SDL_VIDEO_DRIVER_EMSCRIPTEN 1)
    file(GLOB EM_VIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/emscripten/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${EM_VIDEO_SOURCES})
    set(HAVE_SDL_VIDEO TRUE)

    #enable gles
    if(VIDEO_OPENGLES)
      set(SDL_VIDEO_OPENGL_EGL 1)
      set(HAVE_VIDEO_OPENGLES TRUE)
      set(SDL_VIDEO_OPENGL_ES2 1)
      set(SDL_VIDEO_RENDER_OGL_ES2 1)
    endif()
  endif()
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
  if(SDL_AUDIO)
    if(SYSV5 OR SOLARIS OR HPUX)
        set(SDL_AUDIO_DRIVER_SUNAUDIO 1)
        file(GLOB SUN_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/sun/*.c)
        set(SOURCE_FILES ${SOURCE_FILES} ${SUN_AUDIO_SOURCES})
        set(HAVE_SDL_AUDIO TRUE)
    elseif(NETBSD)
        set(SDL_AUDIO_DRIVER_NETBSD 1)
        file(GLOB NETBSD_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/netbsd/*.c)
        set(SOURCE_FILES ${SOURCE_FILES} ${NETBSD_AUDIO_SOURCES})
        set(HAVE_SDL_AUDIO TRUE)
    elseif(AIX)
        set(SDL_AUDIO_DRIVER_PAUDIO 1)
        file(GLOB AIX_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/paudio/*.c)
        set(SOURCE_FILES ${SOURCE_FILES} ${AIX_AUDIO_SOURCES})
        set(HAVE_SDL_AUDIO TRUE)
    endif()
    CheckOSS()
    CheckALSA()
    CheckJACK()
    CheckPulseAudio()
    CheckESD()
    CheckARTS()
    CheckNAS()
    CheckSNDIO()
    CheckFusionSound()
    CheckLibSampleRate()
  endif()

  if(SDL_VIDEO)
    # Need to check for Raspberry PI first and add platform specific compiler flags, otherwise the test for GLES fails!
    CheckRPI()
    CheckX11()
    CheckMir()
    CheckDirectFB()
    CheckOpenGLX11()
    CheckOpenGLESX11()
    CheckWayland()
    CheckVivante()
    CheckKMSDRM()
  endif()

  if(UNIX)
    file(GLOB CORE_UNIX_SOURCES ${SDL2_SOURCE_DIR}/src/core/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${CORE_UNIX_SOURCES})
  endif()

  if(LINUX)
    check_c_source_compiles("
        #include <linux/input.h>
        #ifndef EVIOCGNAME
        #error EVIOCGNAME() ioctl not available
        #endif
        int main(int argc, char** argv) {}" HAVE_INPUT_EVENTS)

    check_c_source_compiles("
        #include <linux/kd.h>
        #include <linux/keyboard.h>

        int main(int argc, char **argv) 
        {
            struct kbentry kbe;
            kbe.kb_table = KG_CTRL;
            ioctl(0, KDGKBENT, &kbe);
        }" HAVE_INPUT_KD)

    file(GLOB CORE_LINUX_SOURCES ${SDL2_SOURCE_DIR}/src/core/linux/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${CORE_LINUX_SOURCES})

    if(HAVE_INPUT_EVENTS)
      set(SDL_INPUT_LINUXEV 1)
    endif()

    if(SDL_HAPTIC AND HAVE_INPUT_EVENTS)
      set(SDL_HAPTIC_LINUX 1)
      file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/linux/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${HAPTIC_SOURCES})
      set(HAVE_SDL_HAPTIC TRUE)
    endif()

    if(HAVE_INPUT_KD)
      set(SDL_INPUT_LINUXKD 1)
    endif()

    check_include_file("libudev.h" HAVE_LIBUDEV_H)

    if(PKG_CONFIG_FOUND)
      pkg_search_module(DBUS dbus-1 dbus)
      if(DBUS_FOUND)
        set(HAVE_DBUS_DBUS_H TRUE)
        include_directories(${DBUS_INCLUDE_DIRS})
        list(APPEND EXTRA_LIBS ${DBUS_LIBRARIES})
      endif()

      pkg_search_module(IBUS ibus-1.0 ibus)
      if(IBUS_FOUND)
        set(HAVE_IBUS_IBUS_H TRUE)
        include_directories(${IBUS_INCLUDE_DIRS})
        list(APPEND EXTRA_LIBS ${IBUS_LIBRARIES})
      endif()
    endif()

    check_include_file("fcitx/frontend.h" HAVE_FCITX_FRONTEND_H)
  endif()

  if(INPUT_TSLIB)
    check_c_source_compiles("
        #include \"tslib.h\"
        int main(int argc, char** argv) { }" HAVE_INPUT_TSLIB)
    if(HAVE_INPUT_TSLIB)
      set(SDL_INPUT_TSLIB 1)
      list(APPEND EXTRA_LIBS ts)
    endif()
  endif()

  if(SDL_JOYSTICK)
    CheckUSBHID()   # seems to be BSD specific - limit the test to BSD only?
    if(LINUX AND NOT ANDROID)
      set(SDL_JOYSTICK_LINUX 1)
      file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/linux/*.c ${SDL2_SOURCE_DIR}/src/joystick/steam/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${JOYSTICK_SOURCES})
      set(HAVE_SDL_JOYSTICK TRUE)
    endif()
  endif()

  CheckPTHREAD()

  if(CLOCK_GETTIME)
    check_library_exists(rt clock_gettime "" FOUND_CLOCK_GETTIME)
    if(FOUND_CLOCK_GETTIME)
      list(APPEND EXTRA_LIBS rt)
      set(HAVE_CLOCK_GETTIME 1)
    else()
      check_library_exists(c clock_gettime "" FOUND_CLOCK_GETTIME)
      if(FOUND_CLOCK_GETTIME)
        set(HAVE_CLOCK_GETTIME 1)
      endif()
    endif()
  endif()

  check_include_file(linux/version.h HAVE_LINUX_VERSION_H)
  if(HAVE_LINUX_VERSION_H)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LINUX_VERSION_H")
  endif()

  if(SDL_POWER)
    if(LINUX)
      set(SDL_POWER_LINUX 1)
      file(GLOB POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/linux/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${POWER_SOURCES})
      set(HAVE_SDL_POWER TRUE)
    endif()
  endif()

  if(SDL_FILESYSTEM)
    set(SDL_FILESYSTEM_UNIX 1)
    file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)
  endif()

  if(SDL_TIMERS)
    set(SDL_TIMER_UNIX 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)
  endif()

  if(RPATH)
    set(SDL_RLD_FLAGS "")
    if(BSDI OR FREEBSD OR LINUX OR NETBSD)
      set(CMAKE_REQUIRED_FLAGS "-Wl,--enable-new-dtags")
      check_c_compiler_flag("" HAVE_ENABLE_NEW_DTAGS)
      set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
      if(HAVE_ENABLE_NEW_DTAGS)
        set(SDL_RLD_FLAGS "-Wl,-rpath,\${libdir} -Wl,--enable-new-dtags")
      else()
        set(SDL_RLD_FLAGS "-Wl,-rpath,\${libdir}")
      endif()
    elseif(SOLARIS)
      set(SDL_RLD_FLAGS "-R\${libdir}")
    endif()
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(HAVE_RPATH TRUE)
  endif()

elseif(WINDOWS)
  find_program(WINDRES windres)

  check_c_source_compiles("
    #include <windows.h>
    int main(int argc, char **argv) { }" HAVE_WIN32_CC)

  file(GLOB CORE_SOURCES ${SDL2_SOURCE_DIR}/src/core/windows/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${CORE_SOURCES})

  if(MSVC)
    # Prevent codegen that would use the VC runtime libraries.
    set_property(DIRECTORY . APPEND PROPERTY COMPILE_OPTIONS "/GS-")
    if(NOT ARCH_64)
      set_property(DIRECTORY . APPEND PROPERTY COMPILE_OPTIONS "/arch:SSE")
    endif()
  endif()

  # Check for DirectX
  if(DIRECTX)
    if(DEFINED MSVC_VERSION AND NOT ${MSVC_VERSION} LESS 1700)
        set(USE_WINSDK_DIRECTX TRUE)
    endif()
    if(NOT CMAKE_COMPILER_IS_MINGW AND NOT USE_WINSDK_DIRECTX)
      if("$ENV{DXSDK_DIR}" STREQUAL "")
        message_error("DIRECTX requires the \$DXSDK_DIR environment variable to be set")
      endif()
      set(CMAKE_REQUIRED_FLAGS "/I\"$ENV{DXSDK_DIR}\\Include\"")
    endif()

    if(HAVE_WIN32_CC)
      # xinput.h may need windows.h, but doesn't include it itself.
      check_c_source_compiles("
        #include <windows.h>
        #include <xinput.h>
        int main(int argc, char **argv) { }" HAVE_XINPUT_H)
      check_c_source_compiles("
        #include <windows.h>
        #include <xinput.h>
        XINPUT_GAMEPAD_EX x1;
        int main(int argc, char **argv) { }" HAVE_XINPUT_GAMEPAD_EX)
      check_c_source_compiles("
        #include <windows.h>
        #include <xinput.h>
        XINPUT_STATE_EX s1;
        int main(int argc, char **argv) { }" HAVE_XINPUT_STATE_EX)
    else()
      check_include_file(xinput.h HAVE_XINPUT_H)
    endif()

    check_include_file(d3d9.h HAVE_D3D_H)
    check_include_file(d3d11_1.h HAVE_D3D11_H)
    check_include_file(ddraw.h HAVE_DDRAW_H)
    check_include_file(dsound.h HAVE_DSOUND_H)
    check_include_file(dinput.h HAVE_DINPUT_H)
    check_include_file(mmdeviceapi.h HAVE_MMDEVICEAPI_H)
    check_include_file(audioclient.h HAVE_AUDIOCLIENT_H)
    check_include_file(dxgi.h HAVE_DXGI_H)
    if(HAVE_D3D_H OR HAVE_D3D11_H OR HAVE_DDRAW_H OR HAVE_DSOUND_H OR HAVE_DINPUT_H)
      set(HAVE_DIRECTX TRUE)
      if(NOT CMAKE_COMPILER_IS_MINGW AND NOT USE_WINSDK_DIRECTX)
      # TODO: change $ENV{DXSDL_DIR} to get the path from the include checks
        link_directories($ENV{DXSDK_DIR}\\lib\\${PROCESSOR_ARCH})
        include_directories($ENV{DXSDK_DIR}\\Include)
      endif()
    endif()
    set(CMAKE_REQUIRED_FLAGS ${ORIG_CMAKE_REQUIRED_FLAGS})
  endif()

  if(SDL_AUDIO)
    set(SDL_AUDIO_DRIVER_WINMM 1)
    file(GLOB WINMM_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/winmm/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${WINMM_AUDIO_SOURCES})
    set(HAVE_SDL_AUDIO TRUE)

    if(HAVE_DSOUND_H)
      set(SDL_AUDIO_DRIVER_DSOUND 1)
      file(GLOB DSOUND_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/directsound/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${DSOUND_AUDIO_SOURCES})
    endif()

    if(HAVE_AUDIOCLIENT_H AND HAVE_MMDEVICEAPI_H)
      set(SDL_AUDIO_DRIVER_WASAPI 1)
      file(GLOB WASAPI_AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/wasapi/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${WASAPI_AUDIO_SOURCES})
    endif()
  endif()

  if(SDL_VIDEO)
    # requires SDL_LOADSO on Windows (IME, DX, etc.)
    if(NOT SDL_LOADSO)
      message_error("SDL_VIDEO requires SDL_LOADSO, which is not enabled")
    endif()
    set(SDL_VIDEO_DRIVER_WINDOWS 1)
    file(GLOB WIN_VIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${WIN_VIDEO_SOURCES})

    if(RENDER_D3D AND HAVE_D3D_H)
      set(SDL_VIDEO_RENDER_D3D 1)
      set(HAVE_RENDER_D3D TRUE)
    endif()
    if(RENDER_D3D AND HAVE_D3D11_H)
      set(SDL_VIDEO_RENDER_D3D11 1)
      set(HAVE_RENDER_D3D TRUE)
    endif()
    set(HAVE_SDL_VIDEO TRUE)
  endif()

  if(SDL_THREADS)
    set(SDL_THREAD_WINDOWS 1)
    set(SOURCE_FILES ${SOURCE_FILES}
      ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_sysmutex.c
      ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_syssem.c
      ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_systhread.c
      ${SDL2_SOURCE_DIR}/src/thread/windows/SDL_systls.c
      ${SDL2_SOURCE_DIR}/src/thread/generic/SDL_syscond.c)
    set(HAVE_SDL_THREADS TRUE)
  endif()

  if(SDL_POWER)
    set(SDL_POWER_WINDOWS 1)
    set(SOURCE_FILES ${SOURCE_FILES} ${SDL2_SOURCE_DIR}/src/power/windows/SDL_syspower.c)
    set(HAVE_SDL_POWER TRUE)
  endif()

  if(SDL_FILESYSTEM)
    set(SDL_FILESYSTEM_WINDOWS 1)
    file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)
  endif()

  # Libraries for Win32 native and MinGW
  list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid)

  # TODO: in configure.in the check for timers is set on
  # cygwin | mingw32* - does this include mingw32CE?
  if(SDL_TIMERS)
    set(SDL_TIMER_WINDOWS 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)
  endif()

  if(SDL_LOADSO)
    set(SDL_LOADSO_WINDOWS 1)
    file(GLOB LOADSO_SOURCES ${SDL2_SOURCE_DIR}/src/loadso/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${LOADSO_SOURCES})
    set(HAVE_SDL_LOADSO TRUE)
  endif()

  file(GLOB CORE_SOURCES ${SDL2_SOURCE_DIR}/src/core/windows/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${CORE_SOURCES})

  if(SDL_VIDEO)
    if(VIDEO_OPENGL)
      set(SDL_VIDEO_OPENGL 1)
      set(SDL_VIDEO_OPENGL_WGL 1)
      set(SDL_VIDEO_RENDER_OGL 1)
      set(HAVE_VIDEO_OPENGL TRUE)
    endif()

    if(VIDEO_OPENGLES)
      set(SDL_VIDEO_OPENGL_EGL 1)
      set(SDL_VIDEO_OPENGL_ES2 1)
      set(SDL_VIDEO_RENDER_OGL_ES2 1)
      set(HAVE_VIDEO_OPENGLES TRUE)
    endif()
  endif()

  if(SDL_JOYSTICK)
    file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/windows/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${JOYSTICK_SOURCES})
    if(HAVE_DINPUT_H)
      set(SDL_JOYSTICK_DINPUT 1)
      list(APPEND EXTRA_LIBS dinput8)
      if(CMAKE_COMPILER_IS_MINGW)
        list(APPEND EXTRA_LIBS dxerr8)
      elseif (NOT USE_WINSDK_DIRECTX)
        list(APPEND EXTRA_LIBS dxerr)
      endif()
    endif()
    if(HAVE_XINPUT_H)
      set(SDL_JOYSTICK_XINPUT 1)
    endif()
    if(NOT HAVE_DINPUT_H AND NOT HAVE_XINPUT_H)
      set(SDL_JOYSTICK_WINMM 1)
    endif()
    set(HAVE_SDL_JOYSTICK TRUE)

    if(SDL_HAPTIC)
      if(HAVE_DINPUT_H OR HAVE_XINPUT_H)
        file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/windows/*.c)
        if(HAVE_DINPUT_H)
          set(SDL_HAPTIC_DINPUT 1)
        endif()
        if(HAVE_XINPUT_H)
          set(SDL_HAPTIC_XINPUT 1)
        endif()
      else()
        file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/dummy/*.c)
        set(SDL_HAPTIC_DUMMY 1)
      endif()
      set(SOURCE_FILES ${SOURCE_FILES} ${HAPTIC_SOURCES})
      set(HAVE_SDL_HAPTIC TRUE)
    endif()
  endif()

  file(GLOB VERSION_SOURCES ${SDL2_SOURCE_DIR}/src/main/windows/*.rc)
  file(GLOB SDLMAIN_SOURCES ${SDL2_SOURCE_DIR}/src/main/windows/*.c)
  if(MINGW OR CYGWIN)
    list(APPEND EXTRA_LIBS mingw32)
    list(APPEND EXTRA_LDFLAGS "-mwindows")
    set(SDL_CFLAGS "${SDL_CFLAGS} -Dmain=SDL_main")
    list(APPEND SDL_LIBS "-lmingw32" "-lSDL2main" "-mwindows")
  endif()
elseif(APPLE)
  # TODO: rework this all for proper MacOS X, iOS and Darwin support

  # We always need these libs on macOS at the moment.
  # !!! FIXME: we need Carbon for some very old API calls in
  # !!! FIXME:  src/video/cocoa/SDL_cocoakeyboard.c, but we should figure out
  # !!! FIXME:  how to dump those.
  if(NOT IOS)
    set(SDL_FRAMEWORK_COCOA 1)
    set(SDL_FRAMEWORK_CARBON 1)
  endif()

  # Requires the darwin file implementation
  if(SDL_FILE)
    file(GLOB EXTRA_SOURCES ${SDL2_SOURCE_DIR}/src/file/cocoa/*.m)
    set(SOURCE_FILES ${EXTRA_SOURCES} ${SOURCE_FILES})
    # !!! FIXME: modern CMake doesn't need "LANGUAGE C" for Objective-C.
    set_source_files_properties(${EXTRA_SOURCES} PROPERTIES LANGUAGE C)
    set(HAVE_SDL_FILE TRUE)
    # !!! FIXME: why is COREVIDEO inside this if() block?
    set(SDL_FRAMEWORK_COREVIDEO 1)
  else()
    message_error("SDL_FILE must be enabled to build on MacOS X")
  endif()

  if(SDL_AUDIO)
    set(SDL_AUDIO_DRIVER_COREAUDIO 1)
    file(GLOB AUDIO_SOURCES ${SDL2_SOURCE_DIR}/src/audio/coreaudio/*.m)
    # !!! FIXME: modern CMake doesn't need "LANGUAGE C" for Objective-C.
    set_source_files_properties(${AUDIO_SOURCES} PROPERTIES LANGUAGE C)
    set(SOURCE_FILES ${SOURCE_FILES} ${AUDIO_SOURCES})
    set(HAVE_SDL_AUDIO TRUE)
    set(SDL_FRAMEWORK_COREAUDIO 1)
    set(SDL_FRAMEWORK_AUDIOTOOLBOX 1)
  endif()

  if(SDL_JOYSTICK)
    set(SDL_JOYSTICK_IOKIT 1)
    if (IOS)
      file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/iphoneos/*.m ${SDL2_SOURCE_DIR}/src/joystick/steam/*.c)
    else()
      file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/darwin/*.c)
    endif()
    set(SOURCE_FILES ${SOURCE_FILES} ${JOYSTICK_SOURCES})
    set(HAVE_SDL_JOYSTICK TRUE)
    set(SDL_FRAMEWORK_IOKIT 1)
    set(SDL_FRAMEWORK_FF 1)
  endif()

  if(SDL_HAPTIC)
    set(SDL_HAPTIC_IOKIT 1)
    if (IOS)
      file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/dummy/*.c)
      set(SDL_HAPTIC_DUMMY 1)
    else()
      file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/darwin/*.c)
    endif()
    set(SOURCE_FILES ${SOURCE_FILES} ${HAPTIC_SOURCES})
    set(HAVE_SDL_HAPTIC TRUE)
    set(SDL_FRAMEWORK_IOKIT 1)
    set(SDL_FRAMEWORK_FF 1)
    if(NOT SDL_JOYSTICK)
      message(FATAL_ERROR "SDL_HAPTIC requires SDL_JOYSTICK to be enabled")
    endif()
  endif()

  if(SDL_POWER)
    set(SDL_POWER_MACOSX 1)
    if (IOS)
      file(GLOB POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/uikit/*.m)
    else()
      file(GLOB POWER_SOURCES ${SDL2_SOURCE_DIR}/src/power/macosx/*.c)
    endif()
    set(SOURCE_FILES ${SOURCE_FILES} ${POWER_SOURCES})
    set(HAVE_SDL_POWER TRUE)
    set(SDL_FRAMEWORK_IOKIT 1)
  endif()

  if(SDL_TIMERS)
    set(SDL_TIMER_UNIX 1)
    file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/unix/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
    set(HAVE_SDL_TIMERS TRUE)
  endif(SDL_TIMERS)

  if(SDL_FILESYSTEM)
    set(SDL_FILESYSTEM_COCOA 1)
    file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/cocoa/*.m)
    # !!! FIXME: modern CMake doesn't need "LANGUAGE C" for Objective-C.
    set_source_files_properties(${FILESYSTEM_SOURCES} PROPERTIES LANGUAGE C)
    set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)
  endif()

  # Actually load the frameworks at the end so we don't duplicate include.
  if(SDL_FRAMEWORK_COREVIDEO)
    find_library(COREVIDEO CoreVideo)
    list(APPEND EXTRA_LIBS ${COREVIDEO})
  endif()
  if(SDL_FRAMEWORK_COCOA)
    find_library(COCOA_LIBRARY Cocoa)
    list(APPEND EXTRA_LIBS ${COCOA_LIBRARY})
  endif()
  if(SDL_FRAMEWORK_IOKIT)
    find_library(IOKIT IOKit)
    list(APPEND EXTRA_LIBS ${IOKIT})
  endif()
  if(SDL_FRAMEWORK_FF)
    find_library(FORCEFEEDBACK ForceFeedback)
    list(APPEND EXTRA_LIBS ${FORCEFEEDBACK})
  endif()
  if(SDL_FRAMEWORK_CARBON)
    find_library(CARBON_LIBRARY Carbon)
    list(APPEND EXTRA_LIBS ${CARBON_LIBRARY})
  endif()
  if(SDL_FRAMEWORK_COREAUDIO)
    find_library(COREAUDIO CoreAudio)
    list(APPEND EXTRA_LIBS ${COREAUDIO})
  endif()
  if(SDL_FRAMEWORK_AUDIOTOOLBOX)
    find_library(AUDIOTOOLBOX AudioToolbox)
    list(APPEND EXTRA_LIBS ${AUDIOTOOLBOX})
  endif()

  # iOS hack needed - http://code.google.com/p/ios-cmake/ ?
  if(SDL_VIDEO)
    if (IOS)
      set(SDL_VIDEO_DRIVER_UIKIT 1)
      file(GLOB UIKITVIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/uikit/*.m)
      set(SOURCE_FILES ${SOURCE_FILES} ${UIKITVIDEO_SOURCES})
    else()
      CheckCOCOA()
      if(VIDEO_OPENGL)
        set(SDL_VIDEO_OPENGL 1)
        set(SDL_VIDEO_OPENGL_CGL 1)
        set(SDL_VIDEO_RENDER_OGL 1)
        set(HAVE_VIDEO_OPENGL TRUE)
      endif()

      if(VIDEO_OPENGLES)
        set(SDL_VIDEO_OPENGL_EGL 1)
        set(SDL_VIDEO_OPENGL_ES2 1)
        set(SDL_VIDEO_RENDER_OGL_ES2 1)
        set(HAVE_VIDEO_OPENGLES TRUE)
      endif()
    endif()
  endif()

  CheckPTHREAD()
elseif(HAIKU)
  if(SDL_VIDEO)
    set(SDL_VIDEO_DRIVER_HAIKU 1)
    file(GLOB HAIKUVIDEO_SOURCES ${SDL2_SOURCE_DIR}/src/video/haiku/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${HAIKUVIDEO_SOURCES})
    set(HAVE_SDL_VIDEO TRUE)

    set(SDL_FILESYSTEM_HAIKU 1)
    file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/haiku/*.cc)
    set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
    set(HAVE_SDL_FILESYSTEM TRUE)

    if(SDL_TIMERS)
      set(SDL_TIMER_HAIKU 1)
      file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/haiku/*.c)
      set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
      set(HAVE_SDL_TIMERS TRUE)
    endif(SDL_TIMERS)

    if(VIDEO_OPENGL)
      # TODO: Use FIND_PACKAGE(OpenGL) instead
      set(SDL_VIDEO_OPENGL 1)
      set(SDL_VIDEO_OPENGL_BGL 1)
      set(SDL_VIDEO_RENDER_OGL 1)
      list(APPEND EXTRA_LIBS GL)
      set(HAVE_VIDEO_OPENGL TRUE)
    endif()
  endif()

  CheckPTHREAD()
endif()

if(VIDEO_VULKAN)
  set(SDL_VIDEO_VULKAN 1)
endif()

# Dummies
# configure.in does it differently:
# if not have X
#   if enable_X {  SDL_X_DISABLED = 1 }
#   [add dummy sources]
# so it always adds a dummy, without checking, if it was actually requested.
# This leads to missing internal references on building, since the
# src/X/*.c does not get included.
if(NOT HAVE_SDL_JOYSTICK)
  set(SDL_JOYSTICK_DISABLED 1)
  if(SDL_JOYSTICK AND NOT APPLE) # results in unresolved symbols on OSX

    file(GLOB JOYSTICK_SOURCES ${SDL2_SOURCE_DIR}/src/joystick/dummy/*.c)
    set(SOURCE_FILES ${SOURCE_FILES} ${JOYSTICK_SOURCES})
  endif()
endif()
if(NOT HAVE_SDL_HAPTIC)
  set(SDL_HAPTIC_DISABLED 1)
  file(GLOB HAPTIC_SOURCES ${SDL2_SOURCE_DIR}/src/haptic/dummy/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${HAPTIC_SOURCES})
endif()
if(NOT HAVE_SDL_LOADSO)
  set(SDL_LOADSO_DISABLED 1)
  file(GLOB LOADSO_SOURCES ${SDL2_SOURCE_DIR}/src/loadso/dummy/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${LOADSO_SOURCES})
endif()
if(NOT HAVE_SDL_FILESYSTEM)
  set(SDL_FILESYSTEM_DISABLED 1)
  file(GLOB FILESYSTEM_SOURCES ${SDL2_SOURCE_DIR}/src/filesystem/dummy/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${FILESYSTEM_SOURCES})
endif()

# We always need to have threads and timers around
if(NOT HAVE_SDL_THREADS)
  set(SDL_THREADS_DISABLED 1)
  file(GLOB THREADS_SOURCES ${SDL2_SOURCE_DIR}/src/thread/generic/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${THREADS_SOURCES})
endif()
if(NOT HAVE_SDL_TIMERS)
  set(SDL_TIMERS_DISABLED 1)
  file(GLOB TIMER_SOURCES ${SDL2_SOURCE_DIR}/src/timer/dummy/*.c)
  set(SOURCE_FILES ${SOURCE_FILES} ${TIMER_SOURCES})
endif()

if(NOT SDLMAIN_SOURCES)
  file(GLOB SDLMAIN_SOURCES ${SDL2_SOURCE_DIR}/src/main/dummy/*.c)
endif()

# Append the -MMD -MT flags
# if(DEPENDENCY_TRACKING)
#   if(COMPILER_IS_GNUCC)
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -MMD -MT \$@")
#   endif()
# endif()

configure_file("${SDL2_SOURCE_DIR}/include/SDL_config.h.cmake"
  "${SDL2_BINARY_DIR}/include/SDL_config.h")

# Prepare the flags and remove duplicates
if(EXTRA_LDFLAGS)
  list(REMOVE_DUPLICATES EXTRA_LDFLAGS)
endif()
if(EXTRA_LIBS)
  list(REMOVE_DUPLICATES EXTRA_LIBS)
endif()
if(EXTRA_CFLAGS)
  list(REMOVE_DUPLICATES EXTRA_CFLAGS)
endif()
listtostr(EXTRA_CFLAGS _EXTRA_CFLAGS)
set(EXTRA_CFLAGS ${_EXTRA_CFLAGS})

# Compat helpers for the configuration files
if(NOT WINDOWS OR CYGWIN)
  # TODO: we need a Windows script, too
  execute_process(COMMAND sh ${SDL2_SOURCE_DIR}/build-scripts/updaterev.sh)

  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "\${prefix}")
  set(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
  set(bindir "\${exec_prefix}/bin")
  set(includedir "\${prefix}/include")
  if(SDL_STATIC)
    set(ENABLE_STATIC_TRUE "")
    set(ENABLE_STATIC_FALSE "#")
  else()
    set(ENABLE_STATIC_TRUE "#")
    set(ENABLE_STATIC_FALSE "")
  endif()
  if(SDL_SHARED)
    set(ENABLE_SHARED_TRUE "")
    set(ENABLE_SHARED_FALSE "#")
  else()
    set(ENABLE_SHARED_TRUE "#")
    set(ENABLE_SHARED_FALSE "")
  endif()

  # Clean up the different lists
  listtostr(EXTRA_LIBS _EXTRA_LIBS "-l")
  set(SDL_STATIC_LIBS ${SDL_LIBS} ${EXTRA_LDFLAGS} ${_EXTRA_LIBS})
  list(REMOVE_DUPLICATES SDL_STATIC_LIBS)
  listtostr(SDL_STATIC_LIBS _SDL_STATIC_LIBS)
  set(SDL_STATIC_LIBS ${_SDL_STATIC_LIBS})
  listtostr(SDL_LIBS _SDL_LIBS)
  set(SDL_LIBS ${_SDL_LIBS})

  # MESSAGE(STATUS "SDL_LIBS: ${SDL_LIBS}")
  # MESSAGE(STATUS "SDL_STATIC_LIBS: ${SDL_STATIC_LIBS}")

  configure_file("${SDL2_SOURCE_DIR}/sdl2.pc.in"
    "${SDL2_BINARY_DIR}/sdl2.pc" @ONLY)
  configure_file("${SDL2_SOURCE_DIR}/sdl2-config.in"
    "${SDL2_BINARY_DIR}/sdl2-config")
  configure_file("${SDL2_SOURCE_DIR}/sdl2-config.in"
    "${SDL2_BINARY_DIR}/sdl2-config" @ONLY)
  configure_file("${SDL2_SOURCE_DIR}/SDL2.spec.in"
    "${SDL2_BINARY_DIR}/SDL2.spec" @ONLY)
endif()

##### Info output #####
message(STATUS "")
message(STATUS "SDL2 was configured with the following options:")
message(STATUS "")
message(STATUS "Platform: ${CMAKE_SYSTEM}")
message(STATUS "64-bit:   ${ARCH_64}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "")
message(STATUS "Subsystems:")
foreach(_SUB ${SDL_SUBSYSTEMS})
  string(TOUPPER ${_SUB} _OPT)
  message_bool_option(${_SUB} SDL_${_OPT})
endforeach()
message(STATUS "")
message(STATUS "Options:")
list(SORT ALLOPTIONS)
foreach(_OPT ${ALLOPTIONS})
  # Longest option is VIDEO_X11_XSCREENSAVER = 22 characters
  # Get the padding
  string(LENGTH ${_OPT} _OPTLEN)
  math(EXPR _PADLEN "23 - ${_OPTLEN}")
  string(RANDOM LENGTH ${_PADLEN} ALPHABET " " _PADDING)
  message_tested_option(${_OPT} ${_PADDING})
endforeach()
message(STATUS "")
message(STATUS " CFLAGS:        ${CMAKE_C_FLAGS}")
message(STATUS " EXTRA_CFLAGS:  ${EXTRA_CFLAGS}")
message(STATUS " EXTRA_LDFLAGS: ${EXTRA_LDFLAGS}")
message(STATUS " EXTRA_LIBS:    ${EXTRA_LIBS}")
message(STATUS "")
message(STATUS " Build Shared Library: ${SDL_SHARED}")
message(STATUS " Build Static Library: ${SDL_STATIC}")
if(SDL_STATIC)
    message(STATUS " Build Static Library with Position Independent Code: ${SDL_STATIC_PIC}")
endif()
message(STATUS "")
if(UNIX)
  message(STATUS "If something was not detected, although the libraries")
  message(STATUS "were installed, then make sure you have set the")
  message(STATUS "CFLAGS and LDFLAGS environment variables correctly.")
  message(STATUS "")
endif()

# Ensure that the extra cflags are used at compile time
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

# Always build SDLmain
add_library(SDL2main STATIC ${SDLMAIN_SOURCES})
target_include_directories(SDL2main PUBLIC $<INSTALL_INTERFACE:include>)
set(_INSTALL_LIBS "SDL2main")
if (NOT ANDROID)
  set_target_properties(SDL2main PROPERTIES DEBUG_POSTFIX ${SDL_CMAKE_DEBUG_POSTFIX})
endif()

if(SDL_SHARED)
  add_library(SDL2 SHARED ${SOURCE_FILES} ${VERSION_SOURCES})
  if(APPLE)
    set_target_properties(SDL2 PROPERTIES MACOSX_RPATH 1)
  elseif(UNIX AND NOT ANDROID)
    set_target_properties(SDL2 PROPERTIES
      VERSION ${LT_VERSION}
      SOVERSION ${LT_REVISION}
      OUTPUT_NAME "SDL2-${LT_RELEASE}")
  else()
    set_target_properties(SDL2 PROPERTIES
      VERSION ${SDL_VERSION}
      SOVERSION ${LT_REVISION}
      OUTPUT_NAME "SDL2")
  endif()
  if(MSVC AND NOT LIBC)
    # Don't try to link with the default set of libraries.
    set_target_properties(SDL2 PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB")
    set_target_properties(SDL2 PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB")
    set_target_properties(SDL2 PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB")
  endif()
  set(_INSTALL_LIBS "SDL2" ${_INSTALL_LIBS})
  target_link_libraries(SDL2 ${EXTRA_LIBS} ${EXTRA_LDFLAGS})
  target_include_directories(SDL2 PUBLIC $<INSTALL_INTERFACE:include>)
  if (NOT ANDROID)
    set_target_properties(SDL2 PROPERTIES DEBUG_POSTFIX ${SDL_CMAKE_DEBUG_POSTFIX})
  endif()
endif()

if(SDL_STATIC)
  set (BUILD_SHARED_LIBS FALSE)
  add_library(SDL2-static STATIC ${SOURCE_FILES})
  if (NOT SDL_SHARED OR NOT WIN32)
    set_target_properties(SDL2-static PROPERTIES OUTPUT_NAME "SDL2")
    # Note: Apparently, OUTPUT_NAME must really be unique; even when
    # CMAKE_IMPORT_LIBRARY_SUFFIX or the like are given. Otherwise
    # the static build may race with the import lib and one will get
    # clobbered, when the suffix is realized via subsequent rename.
  endif()
  set_target_properties(SDL2-static PROPERTIES POSITION_INDEPENDENT_CODE ${SDL_STATIC_PIC})
  if(MSVC AND NOT LIBC)
    set_target_properties(SDL2-static PROPERTIES LINK_FLAGS_RELEASE "/NODEFAULTLIB")
    set_target_properties(SDL2-static PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB")
    set_target_properties(SDL2-static PROPERTIES STATIC_LIBRARY_FLAGS "/NODEFAULTLIB")
  endif()
  # TODO: Win32 platforms keep the same suffix .lib for import and static
  # libraries - do we need to consider this?
  set(_INSTALL_LIBS "SDL2-static" ${_INSTALL_LIBS})
  target_link_libraries(SDL2-static ${EXTRA_LIBS} ${EXTRA_LDFLAGS})
  target_include_directories(SDL2-static PUBLIC $<INSTALL_INTERFACE:include>)
  if (NOT ANDROID)
    set_target_properties(SDL2-static PROPERTIES DEBUG_POSTFIX ${SDL_CMAKE_DEBUG_POSTFIX})
  endif()
endif()

##### Tests #####

if(SDL_TEST)
  file(GLOB TEST_SOURCES ${SDL2_SOURCE_DIR}/src/test/*.c)
  add_library(SDL2_test STATIC ${TEST_SOURCES})
  
  add_subdirectory(test)
endif()

##### Installation targets #####
install(TARGETS ${_INSTALL_LIBS} EXPORT SDL2Targets
  LIBRARY DESTINATION "lib${LIB_SUFFIX}"
  ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
  RUNTIME DESTINATION bin)

##### Export files #####
if (APPLE)
  set(PKG_PREFIX "SDL2.framework/Resources")
elseif (WINDOWS)
  set(PKG_PREFIX "cmake")
else ()
  set(PKG_PREFIX "lib/cmake/SDL2")
endif ()

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_BINARY_DIR}/SDL2ConfigVersion.cmake"
  VERSION ${SDL_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT SDL2Targets
  FILE SDL2Targets.cmake
  NAMESPACE SDL2::
  DESTINATION ${PKG_PREFIX}
)
install(
  FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/SDL2Config.cmake
    ${CMAKE_BINARY_DIR}/SDL2ConfigVersion.cmake
  DESTINATION ${PKG_PREFIX}
  COMPONENT Devel
)

file(GLOB INCLUDE_FILES ${SDL2_SOURCE_DIR}/include/*.h)
file(GLOB BIN_INCLUDE_FILES ${SDL2_BINARY_DIR}/include/*.h)
foreach(_FNAME ${BIN_INCLUDE_FILES})
  get_filename_component(_INCNAME ${_FNAME} NAME)
  list(REMOVE_ITEM INCLUDE_FILES ${SDL2_SOURCE_DIR}/include/${_INCNAME})
endforeach()
list(APPEND INCLUDE_FILES ${BIN_INCLUDE_FILES})
install(FILES ${INCLUDE_FILES} DESTINATION include/SDL2)

if(NOT (WINDOWS OR CYGWIN))
  if(SDL_SHARED)
    if (APPLE)
        set(SOEXT "dylib")
    else()
        set(SOEXT "so")
    endif()
    if(NOT ANDROID)
        install(CODE "
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
          \"libSDL2-2.0.${SOEXT}\" \"libSDL2.${SOEXT}\")")
        install(FILES ${SDL2_BINARY_DIR}/libSDL2.${SOEXT} DESTINATION "lib${LIB_SUFFIX}")
    endif()
  endif()
  if(FREEBSD)
    # FreeBSD uses ${PREFIX}/libdata/pkgconfig
    install(FILES ${SDL2_BINARY_DIR}/sdl2.pc DESTINATION "libdata/pkgconfig")
  else()
    install(FILES ${SDL2_BINARY_DIR}/sdl2.pc
      DESTINATION "lib${LIB_SUFFIX}/pkgconfig")
  endif()
  install(PROGRAMS ${SDL2_BINARY_DIR}/sdl2-config DESTINATION bin)
  # TODO: what about the .spec file? Is it only needed for RPM creation?
  install(FILES "${SDL2_SOURCE_DIR}/sdl2.m4" DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/aclocal")
endif()

##### Uninstall target #####

if(NOT TARGET uninstall)
  configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)

  add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
