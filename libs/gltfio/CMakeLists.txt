cmake_minimum_required(VERSION 3.1)
project(gltfio)

set(TARGET gltfio)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/gltfio/Animator.h
        include/gltfio/AssetLoader.h
        include/gltfio/ResourceLoader.h
        include/gltfio/FilamentAsset.h
)

set(SRCS
        src/Animator.cpp
        src/AssetLoader.cpp
        src/FFilamentAsset.h
        src/FilamentAsset.cpp
        src/GltfEnums.h
        src/MaterialGenerator.cpp
        src/MaterialGenerator.h
        src/ResourceLoader.cpp
        src/Wireframe.cpp
        src/Wireframe.h
        src/math.h
        src/upcast.h
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math utils filamat filament cgltf stb geometry)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)

if (MSVC OR CLANG_CL)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)
