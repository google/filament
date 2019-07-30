cmake_minimum_required(VERSION 3.6)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})

add_library(filament SHARED IMPORTED)
set_target_properties(filament PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilament.a)

add_library(backend SHARED IMPORTED)
set_target_properties(backend PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbackend.a)

add_library(utils SHARED IMPORTED)
set_target_properties(utils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libutils.a)

add_library(filaflat SHARED IMPORTED)
set_target_properties(filaflat PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilaflat.a)

add_library(image STATIC IMPORTED)
set_target_properties(image PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libimage.a)

add_library(ibl STATIC IMPORTED)
set_target_properties(ibl PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libibl.a)

add_library(geometry SHARED IMPORTED)
set_target_properties(geometry PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgeometry.a)

add_library(filabridge SHARED IMPORTED)
set_target_properties(filabridge PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libfilabridge.a)

add_library(gltfio SHARED IMPORTED)
set_target_properties(gltfio PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_core.a)

add_library(gltfio_resources SHARED IMPORTED)
set_target_properties(gltfio_resources PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libgltfio_resources.a)

add_library(bluevk SHARED IMPORTED)
set_target_properties(bluevk PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libbluevk.a)

add_library(smol-v SHARED IMPORTED)
set_target_properties(smol-v PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libsmol-v.a)

include_directories(${FILAMENT_DIR}/include
        ..
        ../../libs/utils/include)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-stack-protector")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math -ffp-contract=fast")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer -ffunction-sections -fdata-sections")

set(CMAKE_SHARED_LINKER_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic-functions")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_SOURCE_DIR}/libgltfio-jni.map")

add_library(gltfio-jni SHARED
        src/main/cpp/AssetLoader.cpp
        src/main/cpp/FilamentAsset.cpp
        src/main/cpp/KtxLoader.cpp
        src/main/cpp/MaterialProvider.cpp
        src/main/cpp/ResourceLoader.cpp
        ../common/CallbackUtils.cpp
        ../common/NioUtils.cpp
)

set_target_properties(gltfio-jni PROPERTIES LINK_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/libgltfio-jni.symbols)

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(gltfio-jni
        android
        gltfio
        gltfio_resources
        filament
        filabridge
        backend
        filaflat
        geometry
        image
        ibl
        utils
        GLESv3
        EGL
        m
        log
)

if (FILAMENT_SUPPORTS_VULKAN)
    target_link_libraries(gltfio-jni bluevk smol-v)
endif()
