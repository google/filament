# Copyright (c) 2018-2025 Cosmin Truta
#
# This software is released under the MIT license. For conditions of
# distribution and use, see the LICENSE file part of this package.

cmake_minimum_required(VERSION 3.14)

project(PNGMINUS C)

option(PNGMINUS_USE_SYSTEM_PNG
       "Use the libpng build found in the system" OFF)

add_executable(png2pnm png2pnm.c)
add_executable(pnm2png pnm2png.c)

if(PNGMINUS_USE_SYSTEM_PNG)
    # Use the system libpng.
    find_package(PNG REQUIRED)
    target_link_libraries(png2pnm PRIVATE PNG::PNG)
    target_link_libraries(pnm2png PRIVATE PNG::PNG)
else()
    # Build and use the internal libpng.
    # Configure libpng for static linking, to produce single-file executables.
    set(PNG_STATIC ON
        CACHE STRING "Build the internal libpng as a static library" FORCE)
    set(PNG_SHARED OFF
        CACHE STRING "Build the internal libpng as a shared library" FORCE)
    set(PNG_FRAMEWORK OFF
        CACHE STRING "Build the internal libpng as a framework bundle" FORCE)
    add_subdirectory(../.. libpng)
    target_include_directories(png2pnm PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libpng>"
    )
    target_include_directories(pnm2png PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libpng>"
    )
    target_link_libraries(png2pnm PRIVATE png_static)
    target_link_libraries(pnm2png PRIVATE png_static)
endif()
