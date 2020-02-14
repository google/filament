cmake_minimum_required(VERSION 3.6)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})
set(GLTFIO_DIR ../../libs/gltfio)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../filament-android ${CMAKE_CURRENT_BINARY_DIR}/filament-android)

add_library(utils STATIC IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(gltfio_resources STATIC IMPORTED)
set_target_properties(gltfio_resources PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_resources.a)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-stack-protector")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -ffp-contract=fast")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -ffunction-sections -fdata-sections")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic-functions")

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.map")

add_library(gltfio-jni SHARED
        ${GLTFIO_DIR}/include/gltfio/Animator.h
        ${GLTFIO_DIR}/include/gltfio/AssetLoader.h
        ${GLTFIO_DIR}/include/gltfio/MaterialProvider.h
        ${GLTFIO_DIR}/include/gltfio/AssetPipeline.h
        ${GLTFIO_DIR}/include/gltfio/ResourceLoader.h
        ${GLTFIO_DIR}/include/gltfio/SimpleViewer.h
        ${GLTFIO_DIR}/include/gltfio/FilamentAsset.h

        ${GLTFIO_DIR}/src/Animator.cpp
        ${GLTFIO_DIR}/src/AssetLoader.cpp
        ${GLTFIO_DIR}/src/DependencyGraph.cpp
        ${GLTFIO_DIR}/src/DependencyGraph.h
        ${GLTFIO_DIR}/src/FFilamentAsset.h
        ${GLTFIO_DIR}/src/FilamentAsset.cpp
        ${GLTFIO_DIR}/src/GltfEnums.h
        ${GLTFIO_DIR}/src/MaterialProvider.cpp
        ${GLTFIO_DIR}/src/ResourceLoader.cpp
        ${GLTFIO_DIR}/src/UbershaderLoader.cpp
        ${GLTFIO_DIR}/src/Wireframe.cpp
        ${GLTFIO_DIR}/src/Wireframe.h
        ${GLTFIO_DIR}/src/math.h
        ${GLTFIO_DIR}/src/upcast.h
        ${GLTFIO_DIR}/src/Image.cpp

        src/main/cpp/Animator.cpp
        src/main/cpp/AssetLoader.cpp
        src/main/cpp/FilamentAsset.cpp
        src/main/cpp/MaterialProvider.cpp
        src/main/cpp/ResourceLoader.cpp
        src/main/cpp/Gltfio.cpp

        ../common/NioUtils.cpp
)

target_include_directories(gltfio-jni PRIVATE
        ..
        ${FILAMENT_DIR}/include
        ${FILAMENT_DIR}/include/gltfio/resources
        ../../third_party/cgltf
        ../../third_party/robin-map
        ../../third_party/stb
        ../../libs/utils/include
)

set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols)

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(gltfio-jni
        filament-jni
        utils
        gltfio_resources
        log
)
