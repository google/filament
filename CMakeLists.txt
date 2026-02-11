# ==================================================================================================
# CMake
# ==================================================================================================
cmake_minimum_required(VERSION 3.22.1)

# Note: we set the main CMakeLists.txt's minimum cmake version to the maximum of all the
# cmake_minimum_required versions for the full build (building for all backends, and for
# both release and debug).

# ==================================================================================================
# Toolchain configuration
# ==================================================================================================
# On iOS, the deployment target is set inside third_party/clang/ios.cmake
if (APPLE AND NOT IOS)
    # This must be set before project() is called
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "")
endif()

# ==================================================================================================
# Project declaration
# ==================================================================================================
project(TNT)

# ==================================================================================================
# Options
# ==================================================================================================
option(FILAMENT_USE_EXTERNAL_GLES3 "Experimental: Compile Filament against OpenGL ES 3" OFF)

option(FILAMENT_ENABLE_LTO "Enable link-time optimizations if supported by the compiler" OFF)
option(FILAMENT_SKIP_SAMPLES "Don't build samples" OFF)

option(FILAMENT_SUPPORTS_XCB "Include XCB support in Linux builds" ON)

option(FILAMENT_SUPPORTS_XLIB "Include XLIB support in Linux builds" ON)

option(FILAMENT_SUPPORTS_EGL_ON_LINUX "Use EGL for OpenGL in Linux builds" OFF)

option(FILAMENT_SUPPORTS_WAYLAND "Include Wayland support in Linux builds" OFF)

option(FILAMENT_SKIP_SDL2 "Skip dependencies of SDL2, and SDL2" OFF)

option(FILAMENT_LINUX_IS_MOBILE "Treat Linux as Mobile" OFF)

option(FILAMENT_ENABLE_ASAN_UBSAN "Enable Address and Undefined Behavior Sanitizers" OFF)

option(FILAMENT_ENABLE_TSAN "Enable Thread Sanitizer" OFF)

option(FILAMENT_ENABLE_COVERAGE "Enable LLVM code coverage" OFF)

option(FILAMENT_ENABLE_FEATURE_LEVEL_0 "Enable Feature Level 0" ON)

option(FILAMENT_SUPPORTS_OSMESA "Enable OSMesa (headless GL context) for Filament" OFF)

option(FILAMENT_ENABLE_FGVIEWER "Enable the frame graph viewer" OFF)

option(FILAMENT_USE_ABSEIL_LOGGING "Use Abseil to log, may increase binary size" OFF)

option(FILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT "Enable GCC support (unsupported)" OFF)

option(FILAMENT_SUPPORTS_WEBP_TEXTURES "Enable webp texture support for Filament (builds libwebp when ON)" OFF)

# This is to disable GTAO for the short-term while we investigate a way to better manage size increases.
# On the regular filament build (where size is of less concern), we enable GTAO by default.
option(FILAMENT_DISABLE_GTAO "Disable GTAO" OFF)

option(FILAMENT_BUILD_TESTING "Build tests" ON)

set(FILAMENT_NDK_VERSION "" CACHE STRING
    "Android NDK version or version prefix to be used when building for Android."
)

set(FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB "3" CACHE STRING
    "Per render pass arena size. Must be roughly 1 MB larger than FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB, default 3."
)

set(FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB "2" CACHE STRING
    "Size of the high-level draw commands buffer. Rule of thumb, 1 MB less than FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB, default 2."
)

set(FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB "2" CACHE STRING
    "Size of the command-stream buffer. As a rule of thumb use the same value as FILAMENT_PER_FRRAME_COMMANDS_SIZE_IN_MB, default 2."
)

set(FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB "4" CACHE STRING
    "Size of the OpenGL handle arena, default 4."
)

set(FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB "8" CACHE STRING
    "Size of the Metal handle arena, default 8."
)

set(FILAMENT_BACKEND_DEBUG_FLAG "" CACHE STRING
    "A debug flag meant for enabling/disabling backend debugging paths"
)

set(FILAMENT_OSMESA_PATH "" CACHE STRING
    "Path to the OSMesa header and lib"
)

# Enable exceptions by default in spirv-cross.
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS OFF)

# ==================================================================================================
# CMake policies
# ==================================================================================================
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    cmake_policy(SET CMP0074 NEW)
endif()

# ==================================================================================================
# Support for ccache
# ==================================================================================================
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    if (WIN32)
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    else()
        set(C_LAUNCHER   "${CCACHE_PROGRAM}")
        set(CXX_LAUNCHER "${CCACHE_PROGRAM}")

        configure_file(build/launch-c.in   launch-c)
        configure_file(build/launch-cxx.in launch-cxx)

        execute_process(COMMAND chmod a+rx
            "${CMAKE_CURRENT_BINARY_DIR}/launch-c"
            "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx"
        )

        if (CMAKE_GENERATOR STREQUAL "Xcode")
            set(CMAKE_XCODE_ATTRIBUTE_CC         "${CMAKE_CURRENT_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_CXX        "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx")
            set(CMAKE_XCODE_ATTRIBUTE_LD         "${CMAKE_CURRENT_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx")
        else()
            set(CMAKE_C_COMPILER_LAUNCHER        "${CMAKE_CURRENT_BINARY_DIR}/launch-c")
            set(CMAKE_CXX_COMPILER_LAUNCHER      "${CMAKE_CURRENT_BINARY_DIR}/launch-cxx")
        endif()
    endif()
endif()

# ==================================================================================================
# Support Vim and Visual Studio Code by generating compile_commands.json
# ==================================================================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================================================================================================
# OS specific
# ==================================================================================================
if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT WEBGL)
    set(LINUX TRUE)
else()
    # since cmake 3.25 LINUX is automatically set based on CMAKE_SYSTEM_NAME, which the android
    # cmake files are setting to "Linux".
    set(LINUX FALSE)
endif()

if (NOT FILAMENT_OSMESA_PATH STREQUAL "")
    if (NOT EXISTS ${FILAMENT_OSMESA_PATH}/)
        message(FATAL_ERROR "Cannot find specified OSMesa build directory: ${FILAMENT_OSMESA_PATH}")
    endif()
    set(FILAMENT_SUPPORTS_OSMESA TRUE)
endif()

if (LINUX)
    if (FILAMENT_SUPPORTS_WAYLAND)
        add_definitions(-DFILAMENT_SUPPORTS_WAYLAND)
        set(FILAMENT_SUPPORTS_X11 FALSE)
    elseif (FILAMENT_SUPPORTS_EGL_ON_LINUX)
        add_definitions(-DFILAMENT_SUPPORTS_EGL_ON_LINUX)
        set(FILAMENT_SUPPORTS_X11 FALSE)
    elseif (FILAMENT_SUPPORTS_OSMESA)
        set(FILAMENT_SUPPORTS_X11 FALSE)
        add_definitions(-DFILAMENT_SUPPORTS_OSMESA)
    else ()
        if (FILAMENT_SUPPORTS_XCB)
            add_definitions(-DFILAMENT_SUPPORTS_XCB)
        endif()

        if (FILAMENT_SUPPORTS_XLIB)
            add_definitions(-DFILAMENT_SUPPORTS_XLIB)
        endif()

        if (FILAMENT_SUPPORTS_XCB OR FILAMENT_SUPPORTS_XLIB)
            add_definitions(-DFILAMENT_SUPPORTS_X11)
            set(FILAMENT_SUPPORTS_X11 TRUE)
        endif()
    endif()
endif()

if (ANDROID OR WEBGL OR IOS OR FILAMENT_LINUX_IS_MOBILE)
    set(IS_MOBILE_TARGET TRUE)
endif()

if (ANDROID)
    add_definitions(-D__ANDROID_UNAVAILABLE_SYMBOLS_ARE_WEAK__)
endif()

if (NOT ANDROID AND NOT WEBGL AND NOT IOS AND NOT FILAMENT_LINUX_IS_MOBILE)
    set(IS_HOST_PLATFORM TRUE)
endif()

if (APPLE)
    if (FILAMENT_SUPPORTS_OSMESA)
        add_definitions(-DFILAMENT_SUPPORTS_OSMESA)
    endif()
endif()

if (WIN32)
    # Link statically against c/c++ lib to avoid missing redistriburable such as
    # "VCRUNTIME140.dll not found. Try reinstalling the app.", but give users
    # a choice to opt for the shared runtime if they want.
    option(USE_STATIC_CRT "Link against the static runtime libraries." ON)

    # On Windows we need to instruct cmake to generate the .def in order to get the .lib required
    # when linking against dlls. CL.EXE will not generate .lib without .def file (or without pragma
    # __declspec(dllexport) in front of each functions).
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    # The CMAKE_CXX_FLAGS vars can be overriden by some Visual Studio generators, so we use an alternative
    # global method here:
    if (${USE_STATIC_CRT})
        add_compile_options(
            $<$<CONFIG:>:/MT>
            $<$<CONFIG:Debug>:/MTd>
            $<$<CONFIG:Release>:/MT>
        )
    else()
        add_compile_options(
            $<$<CONFIG:>:/MD>
            $<$<CONFIG:Debug>:/MDd>
            $<$<CONFIG:Release>:/MD>
        )
    endif()

    # TODO: Figure out why pdb generation messes with incremental compilaton.
    # IN RELEASE_WITH_DEBUG_INFO, generate debug info in .obj, no in pdb.
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Z7")

    # In RELEASE, also generate PDBs.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")

    # In DEBUG, avoid generating a PDB file which seems to mess with incremental compilation.
    # Instead generate debug info directly inside obj files.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")

    # Special settings when building on CI.
    if (${FILAMENT_WINDOWS_CI_BUILD})
        set(LinkerFlags
            CMAKE_SHARED_LINKER_FLAGS_DEBUG
            CMAKE_EXE_LINKER_FLAGS_DEBUG
            CMAKE_MODULE_LINKER_FLAGS_DEBUG
            )
        foreach(LinkerFlag ${LinkerFlags})
            # The /debug flag outputs .pdb files, which we don't need on CI.
            string(REPLACE "/debug" "" ${LinkerFlag} ${${LinkerFlag}})

            # The /INCREMENTAL flag outputs .ilk files for incremental linking. These are huge, and
            # we don't need them on CI.
            string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" ${LinkerFlag} ${${LinkerFlag}})
        endforeach()

        # We turn off compile-time optimizations for CI, as options that speed up the compile-time
        # (e.g. /MP) might increase memory usage, leading to instabilities on limited CI machines.
        option(FILAMENT_SHORTEN_MSVC_COMPILATION "Shorten compile-time in Visual Studio" OFF)
    else()
        option(FILAMENT_SHORTEN_MSVC_COMPILATION "Shorten compile-time in Visual Studio" ON)
    endif()

    if (MSVC)
        if (FILAMENT_SHORTEN_MSVC_COMPILATION)
            # enable multi-processor compilation
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
            # disable run-time STL checks to improve tools (e.g. matc) performance
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_ITERATOR_DEBUG_LEVEL=0")
        endif()
    endif()
endif()

# ==================================================================================================
#  Paths
# ==================================================================================================
# Where our external libs are
set(EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Where our libraries are
set(LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Where our filament code is
set(FILAMENT ${CMAKE_CURRENT_SOURCE_DIR})

# Where our tools are
set(TOOLS ${CMAKE_CURRENT_SOURCE_DIR}/tools)

# ==================================================================================================
# Compiler check
# ==================================================================================================
set(MIN_CLANG_VERSION "6.0")

set(FILAMENT_USING_GCC OFF)
if (CMAKE_C_COMPILER_ID MATCHES "GNU" AND FILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT)
    set(FILAMENT_USING_GCC ON)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_CLANG_VERSION)
        message(FATAL_ERROR "Detected CXX compiler Clang ${CMAKE_CXX_COMPILER_VERSION} < ${MIN_CLANG_VERSION}")
    endif()
elseif (NOT MSVC)
    message(FATAL_ERROR "Detected CXX compiler ${CMAKE_CXX_COMPILER_ID} is unsupported")
endif()

# Detect use of the clang-cl.exe frontend, which does not support all of clangs normal options
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        message(FATAL_ERROR "Building with Clang on Windows is no longer supported. Use MSVC 2019 instead.")
    endif()
endif()

# ==================================================================================================
# Link time optimizations (LTO)
# ==================================================================================================
if (FILAMENT_ENABLE_LTO)
    include(CheckIPOSupported)

    check_ipo_supported(RESULT IPO_SUPPORT)

    if (IPO_SUPPORT)
        message(STATUS "LTO support is enabled")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# ==================================================================================================
# General compiler flags
# ==================================================================================================
set(CXX_STANDARD "-std=c++20")
if (WIN32)
    set(CXX_STANDARD "/std:c++20")
endif()

if (MSVC)
    set(CXX_STANDARD "/std:c++latest")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} /W0 /Zc:__cplusplus")
else()
    if(FILAMENT_USING_GCC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} -Wno-changes-meaning -Wno-return-type -Wno-attributes -Wno-unknown-pragmas -Wno-class-memaccess -Wno-multichar -Wno-deprecated-declarations -Wno-subobject-linkage -Wno-invalid-constexpr")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} -fstrict-aliasing -Wno-unknown-pragmas -Wno-unused-function -Wno-deprecated-declarations")
    endif()
    if (APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nullability-extension")
    endif()
endif()

if (FILAMENT_USE_EXTERNAL_GLES3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFILAMENT_USE_EXTERNAL_GLES3")
endif()

if (FILAMENT_SUPPORTS_EGL_ON_LINUX)
  set(EGL TRUE)
endif()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MATH_DEFINES=1")
endif()

if (LINUX)
    option(USE_STATIC_LIBCXX "Link against the static runtime libraries." ON)

    # Add this step to support both glibc-based Linux distributions (e.g., Ubuntu, Debian)
    # and musl-based distributions (e.g., Alpine).
    include(CheckSymbolExists)
    check_symbol_exists(__GLIBC__ "features.h" CLANG_WITH_GLIBC)

    if (${USE_STATIC_LIBCXX})
        if (FILAMENT_USING_GCC)
            link_libraries("-static-libgcc -static-libstdc++")
        elseif (CLANG_WITH_GLIBC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            link_libraries("-static-libgcc -static-libstdc++")
            link_libraries(libc++.a)
            link_libraries(libc++abi.a)
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
        endif()
    endif()

    # Only linux, clang doesn't want to use a shared library that is not PIC.
    # /usr/bin/ld: ../bluegl/libbluegl.a(BlueGL.cpp.o): relocation R_X86_64_32S
    # against `.bss' can not be used when making a shared object; recompile with -fPIC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if (ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Werror=unguarded-availability")
endif()

if (CYGWIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
    set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON)
endif()

if (MSVC)
    # Since the "secure" replacements that MSVC suggests are not portable, disable
    # the deprecation warnings. Also disable warnings about use of POSIX functions (i.e. "unlink").
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE")
endif()

# Add colors to ninja builds (when not using gcc)
if (UNIX AND CMAKE_GENERATOR STREQUAL "Ninja" AND NOT FILAMENT_USING_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

# Use hidden by default and expose what we need.
if (NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

# ==================================================================================================
# Release compiler flags
# ==================================================================================================
if (NOT MSVC AND NOT IOS)
    # Omitting stack frame pointers prevents the generation of readable stack traces in crash reports on iOS
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
endif()

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -fdata-sections")
endif()

# On Android RELEASE builds, we disable exceptions and RTTI to save some space (about 75 KiB
# saved by -fno-exception and 10 KiB saved by -fno-rtti).
if (ANDROID OR IOS OR WEBGL)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-rtti")
    set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON)

    if (ANDROID OR WEBGL)
        # Omitting unwind info prevents the generation of readable stack traces in crash reports on iOS
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-unwind-tables -fno-asynchronous-unwind-tables")
    endif()
endif()

# Turn off exceptions on iOS debug as well. This fixes an availability error we see when using
# std::visit, which is not supported on iOS 11.0 when exceptions are enabled.
if (IOS)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-exceptions")
    set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON)
endif()

# With WebGL, we disable RTTI even for debug builds because we pass emscripten::val back and forth
# between C++ and JavaScript in order to efficiently access typed arrays, which are unbound.
# NOTE: This is not documented in emscripten so we should consider a different approach.
if (WEBGL)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")
endif()

if (WEBGL_PTHREADS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# ==================================================================================================
# Debug compiler flags
# ==================================================================================================
if (FILAMENT_ENABLE_ASAN_UBSAN)
   set(EXTRA_SANITIZE_OPTIONS "-fsanitize=address -fsanitize=undefined")
endif()
if (FILAMENT_ENABLE_TSAN)
   set(EXTRA_SANITIZE_OPTIONS "-fsanitize=thread")
endif()
if (ANDROID)
    # keep STL debug infos (mimics what the NDK does)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-limit-debug-info")
endif()
if (NOT MSVC AND NOT WEBGL)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector")
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_SANITIZE_OPTIONS}")

if (FILAMENT_ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping -Wno-pass-failed")
endif()

# Disable the stack check for macOS to workaround a known issue in clang 11.0.0.
# See: https://forums.developer.apple.com/thread/121887
if (APPLE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  -fno-stack-check")
endif()

# ==================================================================================================
# Linker flags
# ==================================================================================================
# Strip unused sections
if (NOT WEBGL)
    set(GC_SECTIONS "-Wl,--gc-sections")
endif()

# Prevents stacks from being made for executable by explicitly adding this linker flag. Otherwise,
# it generates warnings newly introduced as of GNU LD 2.39. Modern security practices strongly
# recommend marking the stack as non-executable using the -z noexecstack linker flag to prevent
# stack-based buffer overflow exploits.
# See: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=ffcf9c5700e49c0aee42dcba9a12ba21338e8136
if (LINUX)
    set(NO_EXEC_STACK "-Wl,-z,noexecstack")
endif()

set(B_SYMBOLIC_FUNCTIONS "-Wl,-Bsymbolic-functions")

if (ANDROID)
    set(BINARY_ALIGNMENT "-Wl,-z,max-page-size=16384")
endif()

if (APPLE)
    set(GC_SECTIONS "-Wl,-dead_strip")
    set(B_SYMBOLIC_FUNCTIONS "")

    # tell ranlib to ignore empty compilation units
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    # prevents ar from invoking ranlib, let CMake do it
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GC_SECTIONS} ${NO_EXEC_STACK}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GC_SECTIONS} ${B_SYMBOLIC_FUNCTIONS} ${BINARY_ALIGNMENT}")

if (WEBGL_PTHREADS)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
endif()

# ==================================================================================================
# Project flags
# ==================================================================================================
# Debug modes only
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(TNT_DEV true)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTNT_DEV")
endif()

# By default, build with support for OpenGL on all platforms.
option(FILAMENT_SUPPORTS_OPENGL "Include the OpenGL backend" ON)
if (FILAMENT_SUPPORTS_OPENGL)
    add_definitions(-DFILAMENT_SUPPORTS_OPENGL)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    option(FILAMENT_ENABLE_PERFETTO "Enable perfetto SDK on Android" OFF)
else()
    option(FILAMENT_ENABLE_PERFETTO "Enable perfetto SDK on Android" ON)
endif()
if (FILAMENT_ENABLE_PERFETTO)
    add_definitions(-DFILAMENT_ENABLE_PERFETTO)
endif()

# By default, build with Vulkan support on desktop platforms, although clients must request to use
# it at run time.
if (WIN32 OR WEBGL OR IOS)
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" OFF)
else()
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" ON)
endif()
if (FILAMENT_SUPPORTS_VULKAN)
    add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)
endif()

if (FILAMENT_SUPPORTS_WEBGPU)
    add_definitions(-DFILAMENT_SUPPORTS_WEBGPU)
endif()

if (FILAMENT_SUPPORTS_WEBP_TEXTURES)
    add_definitions(-DFILAMENT_SUPPORTS_WEBP_TEXTURES)
endif()

# Build with Metal support on non-WebGL Apple platforms.
if (APPLE AND NOT WEBGL)
    option(FILAMENT_SUPPORTS_METAL "Include the Metal backend" ON)
else()
    option(FILAMENT_SUPPORTS_METAL "Include the Metal backend" OFF)
endif()
if (FILAMENT_SUPPORTS_METAL)
    add_definitions(-DFILAMENT_SUPPORTS_METAL)
endif()

# Building filamat increases build times and isn't required for web, so turn it off by default.
if (NOT WEBGL)
    option(FILAMENT_BUILD_FILAMAT "Build filamat and JNI buildings" ON)
else()
    option(FILAMENT_BUILD_FILAMAT "Build filamat and JNI buildings" OFF)
endif()

# By default, link in matdbg for Desktop + Debug only since it pulls in filamat and a web server.
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND IS_HOST_PLATFORM)
    option(FILAMENT_ENABLE_MATDBG "Enable the material debugger" ON)
else()
    option(FILAMENT_ENABLE_MATDBG "Enable the material debugger" OFF)
endif()

# Only optimize materials in Release mode (so error message lines match the source code)
if (CMAKE_BUILD_TYPE MATCHES Release)
    option(FILAMENT_DISABLE_MATOPT "Disable material optimizations" OFF)
else()
    option(FILAMENT_DISABLE_MATOPT "Disable material optimizations" ON)
endif()

# Define backend flag for debug only
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT FILAMENT_BACKEND_DEBUG_FLAG STREQUAL "")
    add_definitions(-DFILAMENT_BACKEND_DEBUG_FLAG=${FILAMENT_BACKEND_DEBUG_FLAG})
    unset(FILAMENT_BACKEND_DEBUG_FLAG)
endif()

if (FILAMENT_USE_ABSEIL_LOGGING)
    add_definitions(-DFILAMENT_USE_ABSEIL_LOGGING)
endif()

# ==================================================================================================
# Material compilation flags
# ==================================================================================================

# Target system.
if (IS_MOBILE_TARGET)
    set(MATC_TARGET mobile)
else()
    set(MATC_TARGET desktop)
endif()

set(MATC_API_FLAGS )

if (FILAMENT_SUPPORTS_OPENGL)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a opengl)
endif()
if (FILAMENT_SUPPORTS_VULKAN)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a vulkan)
endif()
if (FILAMENT_SUPPORTS_METAL)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a metal)
endif()

# With WebGPU, push constants are not supported. Skinning uses them.
# WebGPU has a proposal to add push constants at https://github.com/gpuweb/gpuweb/blob/main/proposals/push-constants.md
# With WebGPU, Tint does not support ClipDistance which is used in Stereo.  Mentioned in comment
# https://github.com/google/dawn/blob/855d17b08abdf02f9142bf5a8f14d0ea088810a4/src/tint/lang/spirv/reader/ast_parser/function.cc#L4434
if (FILAMENT_SUPPORTS_WEBGPU)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -a webgpu --variant-filter=stereo)
endif()

# Disable ESSL 1.0 code generation.
if (NOT FILAMENT_ENABLE_FEATURE_LEVEL_0)
    set(MATC_API_FLAGS ${MATC_API_FLAGS} -1)
endif()

# Enable debug info (preserves names in SPIR-V)
if (FILAMENT_ENABLE_MATDBG)
    set(MATC_OPT_FLAGS ${MATC_OPT_FLAGS} -d)
endif()

# Disable optimizations
if (FILAMENT_DISABLE_MATOPT)
    set(MATC_OPT_FLAGS ${MATC_OPT_FLAGS} -g)
endif()

set(MATC_BASE_FLAGS ${MATC_API_FLAGS} -p ${MATC_TARGET} ${MATC_OPT_FLAGS})

# ==================================================================================================
# Distribution
# ==================================================================================================
# choose where to put the objects in the dist folder
if (NOT DIST_ARCH)
    # On Apple silicon, the value of CMAKE_HOST_SYSTEM_PROCESSOR varies based on the CMake process's
    # own architecture. Because of this, running a x86_64 CMake binary on Apple silicon will cause
    # DIST_ARCH to be set incorrectly.
    set(DIST_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")

    if (CMAKE_OSX_ARCHITECTURES MATCHES ".*;.*")
        set(DIST_ARCH "universal")
    else()
        if (NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
            set(DIST_ARCH "${CMAKE_OSX_ARCHITECTURES}")
        endif()
    endif()

endif()
# On Windows machines, the host processor is set to 'AMD64', which we'll interpret as x86_64.
string(TOLOWER "${DIST_ARCH}" DIST_ARCH)
string(REPLACE "amd64" "x86_64" DIST_ARCH "${DIST_ARCH}")
if (NOT DIST_DIR)
    set(DIST_DIR "${DIST_ARCH}")
endif()

# ==================================================================================================
# Functions
# ==================================================================================================
## The MSVC compiler has a limitation on literal string length which is reached when all the
## licenses are concatenated together into a large string... so split them into multiple strings.
function(list_licenses OUTPUT MODULES)
    set(STR_OPENER "R\"FILAMENT__(")
    set(STR_CLOSER ")FILAMENT__\"")
    set(CONTENT)
    set(_MODULES ${MODULES} ${ARGN})
    foreach(module ${_MODULES})
        set(license_path "../../third_party/${module}/LICENSE")
        get_filename_component(fullname "${license_path}" ABSOLUTE)
        if(EXISTS ${fullname})
            string(APPEND CONTENT "${STR_OPENER}License and copyrights for ${module}:\n${STR_CLOSER},\n")
            file(READ ${license_path} license_long)
            string(REPLACE "\n" "${STR_CLOSER},\n${STR_OPENER}" license ${license_long})
            string(APPEND CONTENT ${STR_OPENER}${license}\n${STR_CLOSER},)
            string(APPEND CONTENT "\n\n")
        else()
            message(AUTHOR_WARNING "${license_path} not found. You can ignore this warning if you have devendored ${module}.")
        endif()
    endforeach()
    configure_file(${FILAMENT}/build/licenses.inc.in ${OUTPUT})
endfunction()

set(COMBINE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build/linux/combine-static-libs.sh")

# Add a custom command to TARGET that combines the static libraries in DEPS into a single archive.
function(combine_static_libs TARGET OUTPUT DEPS)
    # Loop through the dependent libraries and query their location on disk.
    set(DEPS_FILES )
    foreach(DEPENDENCY ${DEPS})
        if (TARGET ${DEPENDENCY})
            get_property(dep_type TARGET ${DEPENDENCY} PROPERTY TYPE)
            if (dep_type STREQUAL "STATIC_LIBRARY")
                list(APPEND DEPS_FILES "$<TARGET_FILE:${DEPENDENCY}>")
            endif()
        endif()
    endforeach()

    if (WIN32)
        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND lib.exe /nologo /out:temp.lib ${DEPS_FILES}
            COMMAND "${CMAKE_COMMAND}" -E rename temp.lib ${OUTPUT}
            COMMENT "Combining ${target} dependencies into single shared library"
            VERBATIM
        )
    else()
        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND "${COMBINE_SCRIPT}" "${CMAKE_AR}" "${OUTPUT}" ${DEPS_FILES}
            COMMENT "Combining ${target} dependencies into single shared library"
            VERBATIM
        )
    endif()
endfunction()

# ==================================================================================================
# Configuration importing/exporting prebuilt "/tools" executables
# ==================================================================================================

# In certain cases, we want the ability to separate the build type/flags (release, debug) of tools
# (matc, resgen, etc...) from filament.  Cross compilation is one such case (e.g. building material
# with matc running on host while building filament for Android) [1]. Another example is when client
# wants to enable ASAN for filament but not the tool [2].
#
# Here are the varibles that are used to determine behavior is such flows:
#   - CMAKE_CROSSCOMPILING : Set by cmake to indicate the build's target platform is different
#     from the host platform.
#   - IMPORT_EXECUTABLES_DIR : This is the directory containing the /ImportExecutables-type.cmake
#     which are then included in other CMakeLists.txt to enable finding targets that have been
#     prebuilt. This is set by the client in their `cmake` invocation.
#   - IMPORT_EXECUTABLES : Path to a file to for 1) exporting the prebuilt targets or 2)
#     importing a previously exported targets. (Used internally and should not be set by client).
#   - FILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR : A path set by the client to indicate that
#     they wish to export the tools as prebuilts, and the corresponding targets will be recorded
#     in a cmake file (i.e IMPORT_EXECUTABLES). IMPORT_EXECUTABLES will be set relative to the the
#     given path. This will set FILAMENT_EXPORT_PREBUILT_EXECUTABLES=ON.
#   - FILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR : A path set by the client to indicate that they wish
#     to import the tools as prebuilts from a .cmake file. The location (IMPORT_EXECUTABLES) of the
#     file is relative to the path given. This will set FILAMENT_IMPORT_PREBUILT_EXECUTABLES=ON.
#   - IMPORT_PREBUILT_EXECUTABLES and EXPORT_PREBUILT_EXECUTABLES are internal booleans set to
#     ON based on the prescence of FILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR and
#     FILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR.
#
# In conclusion, for cases
#   - [1] (crosscompiling), the client must set IMPORT_EXECUTABLES_DIR for both when they are building
#     the tools and when they are building the target-platform filament.
#   - [2] (all other instances), the client must set FILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR in
#     the prebuilt exporting step, and set FILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR in the importing
#     step.

# TODO: Fold the cross compilation case into the more general FILAMENT_IMPORT/EXPORT variables.

if (FILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR)
   set(FILAMENT_EXPORT_PREBUILT_EXECUTABLES ON)
   set(IMPORT_EXECUTABLES_DIR ${FILAMENT_EXPORT_PREBUILT_EXECUTABLES_DIR})
endif()

if (FILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR)
   set(FILAMENT_IMPORT_PREBUILT_EXECUTABLES ON)
   set(IMPORT_EXECUTABLES_DIR ${FILAMENT_IMPORT_PREBUILT_EXECUTABLES_DIR})
endif()

if (WEBGL)
    set(IMPORT_EXECUTABLES ${FILAMENT}/${IMPORT_EXECUTABLES_DIR}/ImportExecutables-Release.cmake)
else()
    if (FILAMENT_EXPORT_PREBUILT_EXECUTABLES OR FILAMENT_IMPORT_PREBUILT_EXECUTABLES)
        set(IMPORT_EXECUTABLES_BUILD_TYPE Prebuilt)
    else()
        set(IMPORT_EXECUTABLES_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    endif()
    set(IMPORT_EXECUTABLES ${FILAMENT}/${IMPORT_EXECUTABLES_DIR}/ImportExecutables-${IMPORT_EXECUTABLES_BUILD_TYPE}.cmake)
endif()

# ==================================================================================================
# Common Functions
# ==================================================================================================

# Sets the following variables: RESGEN_HEADER, RESGEN_SOURCE, RESGEN_FLAGS, RESGEN_SOURCE_FLAGS,
# and RESGEN_OUTPUTS. Please pass in an ARCHIVE_NAME that is unique to your project, otherwise the
# incbin directive will happily consume a blob from the wrong project without warnings or errors.
# Also be sure to include the ASM language in the CMake "project" directive for your project.
function(get_resgen_vars ARCHIVE_DIR ARCHIVE_NAME)
    set(OUTPUTS
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.bin
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.S
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.apple.S
        ${ARCHIVE_DIR}/${ARCHIVE_NAME}.h
    )
    if (IOS)
        set(ASM_ARCH_FLAG "-arch ${DIST_ARCH}")
    endif()
    if (APPLE)
        set(ASM_SUFFIX ".apple")
    endif()
    set(RESGEN_HEADER "${ARCHIVE_DIR}/${ARCHIVE_NAME}.h" PARENT_SCOPE)
    # Visual Studio makes it difficult to use assembly without using MASM. MASM doesn't support
    # the equivalent of .incbin, so on Windows we'll just tell resgen to output a C file.
    if (WEBGL OR WIN32 OR ANDROID_ON_WINDOWS)
        set(RESGEN_OUTPUTS "${OUTPUTS};${ARCHIVE_DIR}/${ARCHIVE_NAME}.c" PARENT_SCOPE)
        set(RESGEN_FLAGS -qcx ${ARCHIVE_DIR} -p ${ARCHIVE_NAME} PARENT_SCOPE)
        set(RESGEN_SOURCE "${ARCHIVE_DIR}/${ARCHIVE_NAME}.c" PARENT_SCOPE)
    else()
        set(RESGEN_OUTPUTS "${OUTPUTS}" PARENT_SCOPE)
        set(RESGEN_FLAGS -qx ${ARCHIVE_DIR} -p ${ARCHIVE_NAME} PARENT_SCOPE)
        set(RESGEN_SOURCE "${ARCHIVE_DIR}/${ARCHIVE_NAME}${ASM_SUFFIX}.S" PARENT_SCOPE)
        set(RESGEN_SOURCE_FLAGS "-I'${ARCHIVE_DIR}' ${ASM_ARCH_FLAG}" PARENT_SCOPE)
    endif()
endfunction()

# ==================================================================================================
# Sub-projects
# ==================================================================================================

# Common to all platforms
add_subdirectory(${EXTERNAL}/libgtest/tnt)
add_subdirectory(${LIBRARIES}/camutils)
add_subdirectory(${LIBRARIES}/filabridge)
add_subdirectory(${LIBRARIES}/filaflat)
add_subdirectory(${LIBRARIES}/filagui)
add_subdirectory(${LIBRARIES}/filameshio)
add_subdirectory(${LIBRARIES}/generatePrefilterMipmap)
add_subdirectory(${LIBRARIES}/gltfio)
add_subdirectory(${LIBRARIES}/ibl)
add_subdirectory(${LIBRARIES}/iblprefilter)
add_subdirectory(${LIBRARIES}/image)
add_subdirectory(${LIBRARIES}/imagediff)
add_subdirectory(${LIBRARIES}/ktxreader)
add_subdirectory(${LIBRARIES}/math)
add_subdirectory(${LIBRARIES}/mathio)
add_subdirectory(${LIBRARIES}/uberz)
add_subdirectory(${LIBRARIES}/utils)
add_subdirectory(${LIBRARIES}/viewer)
add_subdirectory(${FILAMENT}/shaders)
add_subdirectory(${EXTERNAL}/abseil/tnt)
# Add zstd before basisu to force it to use the external zstd target,
# preventing a duplicate symbol conflict with its bundled version.
add_subdirectory(${EXTERNAL}/zstd/tnt)
add_subdirectory(${EXTERNAL}/civetweb/tnt)
add_subdirectory(${EXTERNAL}/imgui/tnt)
add_subdirectory(${EXTERNAL}/robin-map/tnt)
add_subdirectory(${EXTERNAL}/smol-v/tnt)
add_subdirectory(${EXTERNAL}/benchmark/tnt)
add_subdirectory(${EXTERNAL}/meshoptimizer/tnt)
add_subdirectory(${EXTERNAL}/mikktspace)
add_subdirectory(${EXTERNAL}/cgltf/tnt)
add_subdirectory(${EXTERNAL}/draco/tnt)
add_subdirectory(${EXTERNAL}/jsmn/tnt)
add_subdirectory(${EXTERNAL}/stb/tnt)
add_subdirectory(${EXTERNAL}/getopt)
add_subdirectory(${EXTERNAL}/perfetto/tnt)
add_subdirectory(${EXTERNAL}/basisu/tnt)

# imageio-lite is needed for viewer
add_subdirectory(${LIBRARIES}/imageio-lite)

# Note that this has to be placed after mikktspace in order for combine_static_libs to work.
add_subdirectory(${LIBRARIES}/geometry)

if (FILAMENT_BUILD_FILAMAT OR IS_HOST_PLATFORM)
    # spirv-tools must come before filamat, as filamat relies on the presence of the
    # spirv-tools_SOURCE_DIR variable.
    set(SPIRV-Headers_SOURCE_DIR ${EXTERNAL}/spirv-headers)
    add_subdirectory(${EXTERNAL}/spirv-tools)
    add_subdirectory(${EXTERNAL}/glslang/tnt)
    add_subdirectory(${EXTERNAL}/spirv-cross/tnt)
    add_subdirectory(${LIBRARIES}/filamat)

    # the material debugger requires filamat
    if (FILAMENT_ENABLE_MATDBG OR IS_HOST_PLATFORM)
        add_subdirectory(${LIBRARIES}/matdbg)
    endif()
endif()

if (FILAMENT_ENABLE_FGVIEWER)
    add_subdirectory(${LIBRARIES}/fgviewer)
endif()

if (FILAMENT_SUPPORTS_WEBP_TEXTURES)
    add_subdirectory(${EXTERNAL}/libwebp/tnt)
endif()

if (FILAMENT_SUPPORTS_VULKAN)
    add_subdirectory(${LIBRARIES}/bluevk)
    add_subdirectory(${EXTERNAL}/vkmemalloc/tnt)
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
    add_subdirectory(${EXTERNAL}/spirv-headers)
endif()

if (FILAMENT_SUPPORTS_WEBGPU)
    add_subdirectory(${EXTERNAL}/dawn/tnt/)
endif()

# Add filament after dawn, since filament/backend depends on dawn targets.
add_subdirectory(${FILAMENT}/filament)

set(FILAMENT_SAMPLES_BINARY_DIR ${PROJECT_BINARY_DIR}/samples)

if (WEBGL)
    add_subdirectory(web/filament-js)
    add_subdirectory(web/samples)
endif()

if (IS_HOST_PLATFORM)
    if (FILAMENT_SUPPORTS_OPENGL)
        add_subdirectory(${LIBRARIES}/bluegl)
    endif()
    if (NOT FILAMENT_SKIP_SDL2)
      add_subdirectory(${LIBRARIES}/filamentapp)
    endif()
    add_subdirectory(${LIBRARIES}/imageio)
    add_subdirectory(${LIBRARIES}/filament-matp)

    add_subdirectory(${FILAMENT}/samples)

    add_subdirectory(${EXTERNAL}/libassimp/tnt)
    add_subdirectory(${EXTERNAL}/libpng/tnt)
    add_subdirectory(${EXTERNAL}/libsdl2/tnt)
    add_subdirectory(${EXTERNAL}/libz/tnt)
    add_subdirectory(${EXTERNAL}/tinyexr/tnt)

    add_subdirectory(${TOOLS}/cmgen)
    add_subdirectory(${TOOLS}/cso-lut)
    add_subdirectory(${TOOLS}/diffimg)
    add_subdirectory(${TOOLS}/filamesh)
    add_subdirectory(${TOOLS}/glslminifier)
    add_subdirectory(${TOOLS}/matc)
    add_subdirectory(${TOOLS}/matinfo)
    if (NOT WIN32)  # matedit not yet supported on Windows
        add_subdirectory(${TOOLS}/matedit)
    endif()
    add_subdirectory(${TOOLS}/mipgen)
    add_subdirectory(${TOOLS}/normal-blending)
    add_subdirectory(${TOOLS}/resgen)
    add_subdirectory(${TOOLS}/rgb-to-lmsr)
    add_subdirectory(${TOOLS}/roughness-prefilter)
    add_subdirectory(${TOOLS}/specular-color)
    add_subdirectory(${TOOLS}/uberz)
    add_subdirectory(${TOOLS}/specgen)
endif()

# Generate exported executables for cross-compiled builds (Android, WebGL, and iOS)
if ((NOT CMAKE_CROSSCOMPILING AND NOT FILAMENT_IMPORT_PREBUILT_EXECUTABLES) OR FILAMENT_EXPORT_PREBUILT_EXECUTABLES)
    export(TARGETS matc cmgen filamesh mipgen resgen uberz glslminifier FILE ${IMPORT_EXECUTABLES})
endif()
