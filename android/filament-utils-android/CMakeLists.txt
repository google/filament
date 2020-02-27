cmake_minimum_required(VERSION 3.6)

set(FILAMENT_DIR ${FILAMENT_DIST_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gltfio-android ${CMAKE_CURRENT_BINARY_DIR}/gltfio-android)

add_library(camutils STATIC IMPORTED)
set_target_properties(camutils PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libcamutils.a)

add_library(image STATIC IMPORTED)
set_target_properties(image PROPERTIES IMPORTED_LOCATION
        ${FILAMENT_DIR}/lib/${ANDROID_ABI}/libimage.a)

include_directories(${FILAMENT_DIR}/include
        ..
        ../../libs/utils/include)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libfilament-utils-jni.map")

add_library(filament-utils-jni SHARED
        src/main/cpp/Bookmark.cpp
        src/main/cpp/Utils.cpp
        src/main/cpp/Manipulator.cpp

        ../common/CallbackUtils.cpp
        ../common/NioUtils.cpp
)

set_target_properties(filament-utils-jni PROPERTIES LINK_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/libfilament-utils-jni.symbols)

# The ordering in the following list is important because CMake does not have dependency information.
target_link_libraries(filament-utils-jni
        gltfio-jni
        camutils
        image
)
