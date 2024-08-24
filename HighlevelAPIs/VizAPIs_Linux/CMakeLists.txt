# ==================================================================================================
# CMake
# ==================================================================================================
cmake_minimum_required(VERSION 3.19)

# ==================================================================================================
# Project declaration
# ==================================================================================================
project(VizAPIs_Linux)

# ==================================================================================================
# CMake policies
# ==================================================================================================
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    cmake_policy(SET CMP0074 NEW)
endif()

# ==================================================================================================
# Compiler check
# ==================================================================================================
set(MIN_CLANG_VERSION "6.0")

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    if (CMAKE_C_COMPILER_VERSION VERSION_LESS MIN_CLANG_VERSION)
        message(FATAL_ERROR "Detected C compiler Clang ${CMAKE_C_COMPILER_VERSION} < ${MIN_CLANG_VERSION}")
    endif()
else()
    message(FATAL_ERROR "Detected C compiler ${CMAKE_C_COMPILER_ID} is unsupported")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_CLANG_VERSION)
        message(FATAL_ERROR "Detected CXX compiler Clang ${CMAKE_CXX_COMPILER_VERSION} < ${MIN_CLANG_VERSION}")
    endif()
else()
    message(FATAL_ERROR "Detected CXX compiler ${CMAKE_CXX_COMPILER_ID} is unsupported")
endif()

# ==================================================================================================
# Support for ccache
# ==================================================================================================
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# ==================================================================================================
# Support Vim and Visual Studio Code by generating compile_commands.json
# ==================================================================================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================================================================================================
# General compiler flags
# ==================================================================================================
set(CXX_STANDARD "-std=c++20")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_STANDARD} -fstrict-aliasing -Wno-unknown-pragmas -Wno-unused-function -Wno-deprecated-declarations -fPIC")

option(USE_STATIC_LIBCXX "Link against the static runtime libraries." ON)
if (${USE_STATIC_LIBCXX})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    link_libraries("-static-libgcc -static-libstdc++")
    link_libraries(libc++.a)
    link_libraries(libc++abi.a)
endif()

# Add colors to ninja builds
if (UNIX AND CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

# Use hidden by default and expose what we need.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# ==================================================================================================
# Release compiler flags
# ==================================================================================================
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -fdata-sections")

# ==================================================================================================
# Debug compiler flags
# ==================================================================================================
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector")

# ==================================================================================================
# Linker flags
# ==================================================================================================
# Strip unused sections
set(GC_SECTIONS "-Wl,--gc-sections")

set(B_SYMBOLIC_FUNCTIONS "-Wl,-Bsymbolic-functions")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GC_SECTIONS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GC_SECTIONS} ${B_SYMBOLIC_FUNCTIONS} ${BINARY_ALIGNMENT}")

# ==================================================================================================
# Project flags
# ==================================================================================================
add_definitions(-DFILAMENT_SUPPORTS_OPENGL)

add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)

add_definitions(-DFILAMENT_DISABLE_MATOPT=1)

add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)

add_definitions(-DFILAMENT_IBL_LITE=1)

add_definitions(-DGLTFIO_DRACO_SUPPORTED=1)

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(COMPONENT_HDRS
        ../API_SOURCE/components/VzActor.h
        ../API_SOURCE/components/VzAsset.h
        ../API_SOURCE/components/VzCamera.h
        ../API_SOURCE/components/VzGeometry.h
        ../API_SOURCE/components/VzLight.h
        ../API_SOURCE/components/VzMaterial.h
        ../API_SOURCE/components/VzMI.h
        ../API_SOURCE/components/VzRenderer.h
        ../API_SOURCE/components/VzScene.h
        ../API_SOURCE/components/VzSkeleton.h
        ../API_SOURCE/components/VzTexture.h
)

set(ROOT_HDRS
        ../API_SOURCE/VizComponentAPIs.h
        ../API_SOURCE/VizEngineAPIs.h
        ../API_SOURCE/VzComponents.h
)

set(PUBLIC_HDRS
        ${COMPONENT_HDRS}
        ${ROOT_HDRS}
)

set(SRCS
        ../API_SOURCE/backend/VzAssetExporter.cpp
        ../API_SOURCE/backend/VzAssetLoader.cpp
        ../API_SOURCE/components/VzActor.cpp
        ../API_SOURCE/components/VzAsset.cpp
        ../API_SOURCE/components/VzCamera.cpp
        ../API_SOURCE/components/VzGeometry.cpp
        ../API_SOURCE/components/VzLight.cpp
        ../API_SOURCE/components/VzMaterial.cpp
        ../API_SOURCE/components/VzMI.cpp
        ../API_SOURCE/components/VzRenderer.cpp
        ../API_SOURCE/components/VzScene.cpp
        ../API_SOURCE/components/VzSkeleton.cpp
        ../API_SOURCE/components/VzTexture.cpp
        ../API_SOURCE/VizComponentAPIs.cpp
        ../API_SOURCE/VizCoreUtils.cpp
        ../API_SOURCE/VizEngineAPIs.cpp
        ../API_SOURCE/VzEngineApp.cpp
        ../API_SOURCE/VzRenderPath.cpp
)

set(PRIVATE_HDRS
        ../API_SOURCE/backend/VzAssetExporter.h
        ../API_SOURCE/backend/VzAssetLoader.h
        ../API_SOURCE/FIncludes.h
        ../API_SOURCE/VizCoreUtils.h
        ../API_SOURCE/VzEngineApp.h
        ../API_SOURCE/VzNameComponents.hpp
        ../API_SOURCE/VzRenderPath.h
)

# ==================================================================================================
# Includes & target definition
# ==================================================================================================
# specify where our headers are
include_directories(../API_SOURCE)

# we're building a library
add_library(${PROJECT_NAME} SHARED ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ../../libs/filabridge/include
    ../../filament/backend/include
    ../../filament/src
    ../../third_party/cgltf
    ../../out/${CMAKE_BUILD_TYPE}/filament/include
    ../../libs/bluevk/include
    ../../libs/utils/include
    ../../libs/filamentapp/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d)

# ==================================================================================================
# Dependencies
# ==================================================================================================

set(FILAMENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../out/${CMAKE_BUILD_TYPE}/filament)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${FILAMENT_DIR}/lib/x86_64/libbackend.a
    ${FILAMENT_DIR}/lib/x86_64/libbasis_transcoder.a
    ${FILAMENT_DIR}/lib/x86_64/libbluegl.a
    ${FILAMENT_DIR}/lib/x86_64/libbluevk.a
    ${FILAMENT_DIR}/lib/x86_64/libcamutils.a
    ${FILAMENT_DIR}/lib/x86_64/libcivetweb.a
    ${FILAMENT_DIR}/lib/x86_64/libdracodec.a
    ${FILAMENT_DIR}/lib/x86_64/libfilabridge.a
    ${FILAMENT_DIR}/lib/x86_64/libfilaflat.a
    ${FILAMENT_DIR}/lib/x86_64/libfilamat.a
    ${FILAMENT_DIR}/lib/x86_64/libfilament.a
    ${FILAMENT_DIR}/lib/x86_64/libfilament-iblprefilter.a
    ${FILAMENT_DIR}/lib/x86_64/libfilameshio.a
    ${FILAMENT_DIR}/lib/x86_64/libgeometry.a
    ${FILAMENT_DIR}/lib/x86_64/libgltfio.a
    ${FILAMENT_DIR}/lib/x86_64/libgltfio_core.a
    ${FILAMENT_DIR}/lib/x86_64/libibl.a
    ${FILAMENT_DIR}/lib/x86_64/libibl-lite.a
    ${FILAMENT_DIR}/lib/x86_64/libimage.a
    ${FILAMENT_DIR}/lib/x86_64/libktxreader.a
    ${FILAMENT_DIR}/lib/x86_64/libmatdbg.a
    ${FILAMENT_DIR}/lib/x86_64/libmeshoptimizer.a
    ${FILAMENT_DIR}/lib/x86_64/libmikktspace.a
    ${FILAMENT_DIR}/lib/x86_64/libshaders.a
    ${FILAMENT_DIR}/lib/x86_64/libsmol-v.a
    ${FILAMENT_DIR}/lib/x86_64/libstb.a
    ${FILAMENT_DIR}/lib/x86_64/libuberarchive.a
    ${FILAMENT_DIR}/lib/x86_64/libuberzlib.a
    ${FILAMENT_DIR}/lib/x86_64/libutils.a
    ${FILAMENT_DIR}/lib/x86_64/libviewer.a
    ${FILAMENT_DIR}/lib/x86_64/libvkshaders.a
    ${FILAMENT_DIR}/lib/x86_64/libzstd.a
)

# ==============================================================================================
# Installation
# ==============================================================================================
install(FILES ${COMPONENT_HDRS} DESTINATION include/components)
install(FILES ${ROOT_HDRS} DESTINATION include)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
