# PNGConfig.cmake
# CMake config file compatible with the FindPNG module.

# Copyright (c) 2024 Cosmin Truta
# Written by Benjamin Buch, 2024
#
# Use, modification and distribution are subject to
# the same licensing terms and conditions as libpng.
# Please see the copyright notice in png.h or visit
# http://libpng.org/pub/png/src/libpng-LICENSE.txt
#
# SPDX-License-Identifier: libpng-2.0

include(CMakeFindDependencyMacro)

find_dependency(ZLIB REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/PNGTargets.cmake")

if(NOT TARGET PNG::PNG)
  if(TARGET PNG::png_shared)
    add_library(PNG::PNG INTERFACE IMPORTED)
    target_link_libraries(PNG::PNG INTERFACE PNG::png_shared)
  elseif(TARGET PNG::png_static)
    add_library(PNG::PNG INTERFACE IMPORTED)
    target_link_libraries(PNG::PNG INTERFACE PNG::png_static)
  endif()
endif()
