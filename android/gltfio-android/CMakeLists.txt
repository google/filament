cmake_minimum_required(VERSION 3.6)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../filament-android ${CMAKE_CURRENT_BINARY_DIR}/filament-android)

add_library(gltfio STATIC IMPORTED)
set_target_properties(gltfio PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_core.a)

add_library(gltfio_resources STATIC IMPORTED)
set_target_properties(gltfio_resources PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_resources.a)

include_directories(${FILAMENT_DIR}/include
        ..
        ../../libs/utils/include)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map")

add_library(gltfio-jni SHARED
        src/main/cpp/Animator.cpp
        src/main/cpp/AssetLoader.cpp
        src/main/cpp/FilamentAsset.cpp
        src/main/cpp/MaterialProvider.cpp
        src/main/cpp/ResourceLoader.cpp
        src/main/cpp/Gltfio.cpp

        ../common/CallbackUtils.cpp
        ../common/NioUtils.cpp
)

set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols)

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(gltfio-jni
        filament-jni
        gltfio
        gltfio_resources
)
