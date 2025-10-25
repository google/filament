cmake_minimum_required(VERSION 3.19)
project(gltfio-android)

option(FILAMENT_SUPPORTS_VULKAN "Enables Vulkan on Android" OFF)
option(FILAMENT_ENABLE_FGVIEWER "Enables Frame Graph Viewer" OFF)
option(FILAMENT_ENABLE_MATDBG "Enables Material debugger" OFF)
option(FILAMENT_DISABLE_MATOPT "Disables material optimizations" OFF)
option(FILAMENT_SUPPORTS_WEBGPU "Enables WebGPU on Android" OFF)

set(CMAKE_CXX_STANDARD 20)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})
set(DRACO_DIR ../../third_party/draco)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../filament-android ${CMAKE_CURRENT_BINARY_DIR}/filament-android)

add_library(gltfio_core STATIC IMPORTED)
set_target_properties(gltfio_core PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_core.a)

add_library(dracodec STATIC IMPORTED)
set_target_properties(dracodec PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libdracodec.a)

add_library(ktxreader STATIC IMPORTED)
set_target_properties(ktxreader PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libktxreader.a)

add_library(stb STATIC IMPORTED)
set_target_properties(stb PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libstb.a)

add_library(webpdecoder STATIC IMPORTED)
set_target_properties(webpdecoder PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libwebpdecoder.a)

add_library(basis_transcoder STATIC IMPORTED)
set_target_properties(basis_transcoder PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbasis_transcoder.a)

add_library(utils STATIC IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(perfetto STATIC IMPORTED)
set_target_properties(perfetto PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libperfetto.a)

add_library(uberzlib STATIC IMPORTED)
set_target_properties(uberzlib PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libuberzlib.a)

add_library(uberarchive STATIC IMPORTED)
set_target_properties(uberarchive PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libuberarchive.a)

set(VERSION_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${VERSION_SCRIPT}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384")

set(GLTFIO_SRCS
        src/main/cpp/Animator.cpp
        src/main/cpp/AssetLoader.cpp
        src/main/cpp/FilamentAsset.cpp
        src/main/cpp/FilamentInstance.cpp
        src/main/cpp/MaterialKey.cpp
        src/main/cpp/MaterialKey.h
        src/main/cpp/UbershaderProvider.cpp
        src/main/cpp/ResourceLoader.cpp

        ${FILAMENT_DIR}/include/gltfio/materials/uberarchive.h

        ../common/NioUtils.cpp
)

set(GLTFIO_INCLUDE_DIRS
        ..
        ${FILAMENT_DIR}/include
        ${FILAMENT_DIR}/backend/include
        ../../libs/gltfio/include
        ../../third_party/cgltf
        ../../third_party/robin-map
        ../../third_party/stb
        ../../third_party/libwebp/src
        ../../libs/utils/include
        ../../libs/ktxreader/include
)

add_library(gltfio-jni SHARED ${GLTFIO_SRCS})

target_compile_definitions(gltfio-jni PUBLIC GLTFIO_DRACO_SUPPORTED=1)

target_include_directories(gltfio-jni PRIVATE ${GLTFIO_INCLUDE_DIRS})
target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/src)
target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/tnt)

# Force a relink when the version script is changed:
set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols;${VERSION_SCRIPT}")

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(gltfio-jni
        PRIVATE gltfio_core
        PRIVATE ktxreader
        PUBLIC filament-jni
        PRIVATE stb
        PRIVATE basis_transcoder
        PRIVATE uberarchive
        PRIVATE dracodec
        PRIVATE webpdecoder
        PRIVATE utils
        PRIVATE uberzlib
        PRIVATE perfetto        # needed only when FILAMENT_ENABLE_PERFETTO is defined
        PRIVATE log             # needed only when perfetto above is used
)
