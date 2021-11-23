cmake_minimum_required(VERSION 3.19)
project(gltfio-android)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})
set(GLTFIO_DIR ../../libs/gltfio)
set(DRACO_DIR ../../third_party/draco)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../filament-android ${CMAKE_CURRENT_BINARY_DIR}/filament-android)

add_library(dracodec STATIC IMPORTED)
set_target_properties(dracodec PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libdracodec.a)

add_library(utils STATIC IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(gltfio_resources STATIC IMPORTED)
set_target_properties(gltfio_resources PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_resources.a)

add_library(gltfio_resources_lite STATIC IMPORTED)
set_target_properties(gltfio_resources_lite PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_resources_lite.a)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map")

set(GLTFIO_SRCS
        ${GLTFIO_DIR}/include/gltfio/Animator.h
        ${GLTFIO_DIR}/include/gltfio/AssetLoader.h
        ${GLTFIO_DIR}/include/gltfio/MaterialProvider.h
        ${GLTFIO_DIR}/include/gltfio/ResourceLoader.h
        ${GLTFIO_DIR}/include/gltfio/FilamentAsset.h
        ${GLTFIO_DIR}/include/gltfio/FilamentInstance.h

        ${GLTFIO_DIR}/src/Animator.cpp
        ${GLTFIO_DIR}/src/AssetLoader.cpp
        ${GLTFIO_DIR}/src/DracoCache.cpp
        ${GLTFIO_DIR}/src/DracoCache.h
        ${GLTFIO_DIR}/src/DependencyGraph.cpp
        ${GLTFIO_DIR}/src/DependencyGraph.h
        ${GLTFIO_DIR}/src/FFilamentAsset.h
        ${GLTFIO_DIR}/src/FilamentAsset.cpp
        ${GLTFIO_DIR}/src/FFilamentInstance.h
        ${GLTFIO_DIR}/src/FilamentInstance.cpp
        ${GLTFIO_DIR}/src/GltfEnums.h
        ${GLTFIO_DIR}/src/MaterialProvider.cpp
        ${GLTFIO_DIR}/src/MorphHelper.h
        ${GLTFIO_DIR}/src/MorphHelper.cpp
        ${GLTFIO_DIR}/src/ResourceLoader.cpp
        ${GLTFIO_DIR}/src/TangentsJob.h
        ${GLTFIO_DIR}/src/TangentsJob.cpp
        ${GLTFIO_DIR}/src/UbershaderLoader.cpp
        ${GLTFIO_DIR}/src/Wireframe.cpp
        ${GLTFIO_DIR}/src/Wireframe.h
        ${GLTFIO_DIR}/src/math.h
        ${GLTFIO_DIR}/src/upcast.h
        ${GLTFIO_DIR}/src/Image.cpp

        src/main/cpp/Animator.cpp
        src/main/cpp/AssetLoader.cpp
        src/main/cpp/FilamentAsset.cpp
        src/main/cpp/FilamentInstance.cpp
        src/main/cpp/MaterialKey.cpp
        src/main/cpp/MaterialKey.h
        src/main/cpp/UbershaderLoader.cpp
        src/main/cpp/ResourceLoader.cpp

        ${FILAMENT_DIR}/include/gltfio/resources/gltfresources_lite.h
        ${FILAMENT_DIR}/include/gltfio/resources/gltfresources.h

        ../common/NioUtils.cpp
)

set(GLTFIO_INCLUDE_DIRS
        ..
        ${FILAMENT_DIR}/include
        ${FILAMENT_DIR}/include/gltfio/resources
        ../../filament/backend/include
        ../../libs/gltfio/include
        ../../third_party/cgltf
        ../../third_party/robin-map
        ../../third_party/hat-trie
        ../../third_party/stb
        ../../libs/utils/include
)

add_library(gltfio-jni SHARED ${GLTFIO_SRCS})
target_include_directories(gltfio-jni PRIVATE ${GLTFIO_INCLUDE_DIRS})
set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols)
set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map)

if(GLTFIO_LITE)
        target_compile_definitions(gltfio-jni PUBLIC GLTFIO_LITE=1)
        target_link_libraries(gltfio-jni filament-jni utils log gltfio_resources_lite)
else()
        target_link_libraries(gltfio-jni filament-jni utils log gltfio_resources)

        # Enable Draco in the non-lite variant of gltfio.
        target_link_libraries(gltfio-jni dracodec)
        target_compile_definitions(gltfio-jni PUBLIC GLTFIO_DRACO_SUPPORTED=1)
        target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/src)
        target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/tnt)
endif()
