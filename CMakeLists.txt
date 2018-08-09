# ==================================================================================================
# CMake
# ==================================================================================================
cmake_minimum_required(VERSION 3.1)

if (WIN32)
    # On Windows we need to instruct cmake to generate the .def in order to get the .lib required
    # when linking against dlls. CL.EXE will not generate .lib without .def file (or without pragma
    # __declspec(dllexport) in front of each functions).
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# ==================================================================================================
# Project declaration
# ==================================================================================================
project(TNT)

# ==================================================================================================
# Options
# ==================================================================================================

option(ENABLE_JAVA "Compile Java projects, requires a JDK and the JAVA_HOME env var" ON)

# ==================================================================================================
# OS specific
# ==================================================================================================
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX TRUE)
endif()

if (WIN32)
    # TODO: Figure out why pdb generation messes with incremental compilaton.
    # IN RELEASE_WITH_DEBUG_INFO, generate debug info in .obj, no in pdb.
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /Z7")

    # In RELEASE, link statically against c/c++ lib to avoid missing redistriburable such as
    # "VCRUNTIME140.dll not found. Try reinstalling the app.". Also generate PDBs.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /Zi")

    # In DEBUG, avoid generating a PDB file which seems to mess with incremental compilation.
    # Instead generate debug info directly inside obj files.
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
endif()

# ==================================================================================================
#  Paths
# ==================================================================================================
# Where our external libs are
set(EXTERNAL ${CMAKE_SOURCE_DIR}/third_party)

# Where our libraries are
set(LIBRARIES ${CMAKE_SOURCE_DIR}/libs)

# Where our filament code is
set(FILAMENT ${CMAKE_SOURCE_DIR})

# Where our tools are
set(TOOLS ${CMAKE_SOURCE_DIR}/tools)

# ==================================================================================================
# General compiler flags
# ==================================================================================================
set(CXX_STANDARD "-std=c++14")
if (WIN32)
    set(CXX_STANDARD "/std:c++14")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} -fstrict-aliasing -Wno-unknown-pragmas -Wno-unused-function")

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MATH_DEFINES=1")
endif()

if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    if (FILAMENT_REQUIRES_CXXABI)
        # Required in CI environment with custom libc++ and libc++abi
        link_libraries("-lc++abi")
    endif()
    # To distribute our binaries, we must remove the dependency on libc++ and libgcc.
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        link_libraries("-static-libgcc -static-libstdc++")
    endif()
else()
    if (FILAMENT_REQUIRES_CXXABI)
        message("The option FILAMENT_REQUIRES_CXXABI is unsupported on this platform")
    endif()
endif()

if (CYGWIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
endif()

# Add colors to ninja builds
if (UNIX AND CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

# ==================================================================================================
# Release compiler flags
# ==================================================================================================
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -ffunction-sections -fdata-sections")

# On Android RELEASE builds, we disable exceptions and RTTI to save some space (about 75 KiB
# saved by -fno-exception and 10 KiB saved by -fno-rtti).
if (ANDROID)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-rtti")
endif()

# ==================================================================================================
# Debug compiler flags
# ==================================================================================================
# ASAN is deactivated for now because:
#  -fsanitize=undefined causes extremely long link times
#  -fsanitize=address causes a crash with assimp, which we can't explain for now
#set(EXTRA_SANITIZE_OPTIONS "-fsanitize=undefined -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_SANITIZE_OPTIONS}")

# ==================================================================================================
# Linker flags
# ==================================================================================================
# Strip unused sections
set(GC_SECTIONS "--gc-sections")
set(B_SYMBOLIC_FUNCTIONS "-Bsymbolic-functions")

if (APPLE)
    set(GC_SECTIONS "-dead_strip")
    set(B_SYMBOLIC_FUNCTIONS "")

    # tell ranlib to ignore empty compilation units
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
    # prevents ar from invoking ranlib, let CMake do it
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -Wl,${GC_SECTIONS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,${GC_SECTIONS} -Wl,${B_SYMBOLIC_FUNCTIONS}")

# ==================================================================================================
# Project flags
# ==================================================================================================
# Debug modes only
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(TNT_DEV true)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTNT_DEV")
endif()

# By default, build with Vulkan support on desktop platforms, although clients must request to use
# it at run time. On Android, the build does not include Vulkan support unless CMake is invoked
# with -DFILAMENT_SUPPORTS_VULKAN=ON.
# Vulkan is not supported on Windows.
if (ANDROID OR WIN32)
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" OFF)
else()
    option(FILAMENT_SUPPORTS_VULKAN "Include the Vulkan backend" ON)
endif()
if (FILAMENT_SUPPORTS_VULKAN)
    add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)
endif()

# ==================================================================================================
# Distribution
# ==================================================================================================
# choose where to put the objects in the dist folder
if (NOT DIST_ARCH)
    set(DIST_ARCH x86_64)
endif()
if (NOT DIST_DIR)
    set(DIST_DIR "${DIST_ARCH}")
endif()

# ==================================================================================================
# Functions
# ==================================================================================================
function(list_licenses OUTPUT MODULES)
    file(WRITE ${OUTPUT} "R\"FILAMENT__(\n")
    set(_MODULES ${MODULES} ${ARGN})
    foreach(module ${_MODULES})
        set(license_path "../../third_party/${module}/LICENSE")
        get_filename_component(fullname "${license_path}" ABSOLUTE)
        file(APPEND ${OUTPUT} "License and copyrights for ${module}:\n\n")
        file(READ ${license_path} license)
        file(APPEND ${OUTPUT} ${license})
        file(APPEND ${OUTPUT} "\n\n")
    endforeach()
    file(APPEND ${OUTPUT} ")FILAMENT__\"\n")
endfunction(list_licenses)

# ==================================================================================================
# Sub-projects
# ==================================================================================================
# Common to all platforms
add_subdirectory(${EXTERNAL}/libgtest/tnt)
add_subdirectory(${LIBRARIES}/filabridge)
add_subdirectory(${LIBRARIES}/filaflat)
add_subdirectory(${LIBRARIES}/filamat)
add_subdirectory(${LIBRARIES}/image)
add_subdirectory(${LIBRARIES}/math)
add_subdirectory(${LIBRARIES}/utils)
add_subdirectory(${FILAMENT}/filament)
add_subdirectory(${FILAMENT}/shaders)
add_subdirectory(${EXTERNAL}/robin-map/tnt)

if (FILAMENT_SUPPORTS_VULKAN)
    add_subdirectory(${LIBRARIES}/bluevk)
    add_subdirectory(${EXTERNAL}/vkmemalloc/tnt)
endif()

if (APPLE)
    add_subdirectory(${EXTERNAL}/moltenvk/tnt)
endif()

set (FILAMENT_SAMPLES_BINARY_DIR ${PROJECT_BINARY_DIR}/samples)

if (NOT ANDROID)
    add_subdirectory(${FILAMENT}/samples)

    add_subdirectory(${LIBRARIES}/bluegl)
    add_subdirectory(${LIBRARIES}/filagui)
    add_subdirectory(${LIBRARIES}/imageio)

    add_subdirectory(${FILAMENT}/java)

    # must come first because of glslang
    add_subdirectory(${EXTERNAL}/spirv-tools)

    add_subdirectory(${EXTERNAL}/getopt)
    add_subdirectory(${EXTERNAL}/glslang/tnt)
    add_subdirectory(${EXTERNAL}/imgui/tnt)
    add_subdirectory(${EXTERNAL}/libassimp/tnt)
    add_subdirectory(${EXTERNAL}/libpng/tnt)
    add_subdirectory(${EXTERNAL}/libsdl2/tnt)
    add_subdirectory(${EXTERNAL}/libz/tnt)
    add_subdirectory(${EXTERNAL}/skylight/tnt)
    add_subdirectory(${EXTERNAL}/spirv-cross/tnt)
    add_subdirectory(${EXTERNAL}/stb/tnt)
    add_subdirectory(${EXTERNAL}/tinyexr/tnt)

    add_subdirectory(${TOOLS}/cmgen)
    add_subdirectory(${TOOLS}/filamesh)
    add_subdirectory(${TOOLS}/matc)
    add_subdirectory(${TOOLS}/matinfo)
    add_subdirectory(${TOOLS}/normal-blending)
    add_subdirectory(${TOOLS}/roughness-prefilter)
    add_subdirectory(${TOOLS}/skygen)
    add_subdirectory(${TOOLS}/specular-color)
endif()

# Generate exported executables for cross-compiled builds (Android)
if (NOT CMAKE_CROSSCOMPILING)
    export(TARGETS matc cmgen FILE ${CMAKE_SOURCE_DIR}/${IMPORT_EXECUTABLES_DIR}/ImportExecutables-${CMAKE_BUILD_TYPE}.cmake)
endif()
