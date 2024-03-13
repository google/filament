cmake_minimum_required(VERSION 3.19)
project(gltfio-android)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})
set(GLTFIO_DIR ../../libs/gltfio)
set(DRACO_DIR ../../third_party/draco)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../filament-android ${CMAKE_CURRENT_BINARY_DIR}/filament-android)

add_library(dracodec STATIC IMPORTED)
set_target_properties(dracodec PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libdracodec.a)

add_library(meshoptimizer STATIC IMPORTED)
set_target_properties(meshoptimizer PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libmeshoptimizer.a)

add_library(ktxreader STATIC IMPORTED)
set_target_properties(ktxreader PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libktxreader.a)

add_library(stb STATIC IMPORTED)
set_target_properties(stb PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libstb.a)

add_library(basis_transcoder STATIC IMPORTED)
set_target_properties(basis_transcoder PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbasis_transcoder.a)

add_library(zstd STATIC IMPORTED)
set_target_properties(zstd PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libzstd.a)

add_library(utils STATIC IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(uberzlib STATIC IMPORTED)
set_target_properties(uberzlib PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libuberzlib.a)

add_library(uberarchive STATIC IMPORTED)
set_target_properties(uberarchive PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libuberarchive.a)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map")

set(GLTFIO_SRCS
        ${GLTFIO_DIR}/include/gltfio/Animator.h
        ${GLTFIO_DIR}/include/gltfio/AssetLoader.h
        ${GLTFIO_DIR}/include/gltfio/FilamentAsset.h
        ${GLTFIO_DIR}/include/gltfio/FilamentInstance.h
        ${GLTFIO_DIR}/include/gltfio/MaterialProvider.h
        ${GLTFIO_DIR}/include/gltfio/NodeManager.h
        ${GLTFIO_DIR}/include/gltfio/TrsTransformManager.h
        ${GLTFIO_DIR}/include/gltfio/ResourceLoader.h
        ${GLTFIO_DIR}/include/gltfio/TextureProvider.h
        ${GLTFIO_DIR}/include/gltfio/math.h

        ${GLTFIO_DIR}/src/ArchiveCache.cpp
        ${GLTFIO_DIR}/src/ArchiveCache.h
        ${GLTFIO_DIR}/src/Animator.cpp
        ${GLTFIO_DIR}/src/AssetLoader.cpp
        ${GLTFIO_DIR}/src/DependencyGraph.cpp
        ${GLTFIO_DIR}/src/DependencyGraph.h
        ${GLTFIO_DIR}/src/DracoCache.cpp
        ${GLTFIO_DIR}/src/DracoCache.h
        ${GLTFIO_DIR}/src/FFilamentAsset.h
        ${GLTFIO_DIR}/src/FFilamentInstance.h
        ${GLTFIO_DIR}/src/FilamentAsset.cpp
        ${GLTFIO_DIR}/src/FilamentInstance.cpp
        ${GLTFIO_DIR}/src/FNodeManager.h
        ${GLTFIO_DIR}/src/FTrsTransformManager.h
        ${GLTFIO_DIR}/src/GltfEnums.h
        ${GLTFIO_DIR}/src/Ktx2Provider.cpp
        ${GLTFIO_DIR}/src/MaterialProvider.cpp
        ${GLTFIO_DIR}/src/NodeManager.cpp
        ${GLTFIO_DIR}/src/TrsTransformManager.cpp
        ${GLTFIO_DIR}/src/ResourceLoader.cpp
        ${GLTFIO_DIR}/src/StbProvider.cpp
        ${GLTFIO_DIR}/src/TangentsJob.cpp
        ${GLTFIO_DIR}/src/TangentsJob.h
        ${GLTFIO_DIR}/src/UbershaderProvider.cpp
        ${GLTFIO_DIR}/src/Utility.cpp
        ${GLTFIO_DIR}/src/Utility.h
        ${GLTFIO_DIR}/src/Wireframe.cpp
        ${GLTFIO_DIR}/src/Wireframe.h
        ${GLTFIO_DIR}/src/downcast.h
        ${GLTFIO_DIR}/src/extended/AssetLoaderExtended.h
        ${GLTFIO_DIR}/src/extended/TangentsJobExtended.cpp
        ${GLTFIO_DIR}/src/extended/TangentsJobExtended.h
        ${GLTFIO_DIR}/src/extended/TangentSpaceMeshWrapper.cpp
        ${GLTFIO_DIR}/src/extended/TangentSpaceMeshWrapper.h


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
        ../../filament/backend/include
        ../../libs/gltfio/include
        ../../third_party/basisu/zstd
        ../../third_party/cgltf
        ../../third_party/meshoptimizer/src
        ../../third_party/robin-map
        ../../third_party/hat-trie
        ../../third_party/stb
        ../../libs/utils/include
        ../../libs/ktxreader/include
)

add_library(gltfio-jni SHARED ${GLTFIO_SRCS})
target_include_directories(gltfio-jni PRIVATE ${GLTFIO_INCLUDE_DIRS})
set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols)
set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map)
target_link_libraries(gltfio-jni filament-jni utils uberzlib log stb ktxreader basis_transcoder zstd uberarchive)
target_link_libraries(gltfio-jni dracodec meshoptimizer)
target_compile_definitions(gltfio-jni PUBLIC GLTFIO_DRACO_SUPPORTED=1)
target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/src)
target_include_directories(gltfio-jni PRIVATE ${DRACO_DIR}/tnt)
