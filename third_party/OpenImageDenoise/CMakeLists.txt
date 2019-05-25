## ======================================================================== ##
## Copyright 2009-2019 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

cmake_minimum_required(VERSION 3.1)

set(OIDN_VERSION_MAJOR 0)
set(OIDN_VERSION_MINOR 9)
set(OIDN_VERSION_PATCH 0)
set(OIDN_VERSION_NOTE "")

set(OIDN_VERSION ${OIDN_VERSION_MAJOR}.${OIDN_VERSION_MINOR}.${OIDN_VERSION_PATCH})
math(EXPR OIDN_VERSION_NUMBER "10000*${OIDN_VERSION_MAJOR} + 100*${OIDN_VERSION_MINOR} + ${OIDN_VERSION_PATCH}")

project(OpenImageDenoise
  VERSION ${OIDN_VERSION}
  LANGUAGES CXX
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Build as shared or static library
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13.0")
  option(OIDN_STATIC_LIB "Build Open Image Denoise as a static library.")
  mark_as_advanced(CLEAR OIDN_STATIC_LIB)
else()
  set(OIDN_STATIC_LIB OFF CACHE BOOL "Build Open Image Denoise as a static library." FORCE)
  mark_as_advanced(OIDN_STATIC_LIB)
endif()
if(OIDN_STATIC_LIB)
  set(OIDN_LIB_TYPE STATIC)
else()
  set(OIDN_LIB_TYPE SHARED)
endif()

# Configuration types
set(CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
if(win32)
  if(NOT OIDN_DEFAULT_CMAKE_CONFIGURATION_TYPES_SET)
    set(CMAKE_CONFIGURATION_TYPES "${CONFIGURATION_TYPES}"
        CACHE STRING "List of generated configurations." FORCE)
    set(OOIDN_DEFAULT_CMAKE_CONFIGURATION_TYPES_SET ON
        CACHE INTERNAL "Default CMake configuration types set.")
  endif()
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type." FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CONFIGURATION_TYPES})
endif()

# Output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# Configure packaging
include(package)

## ----------------------------------------------------------------------------
## MKL-DNN
## ----------------------------------------------------------------------------

# Configure MKL-DNN
set(MKLDNN_LIBRARY_TYPE "STATIC" CACHE INTERNAL "")
set(MKLDNN_THREADING "TBB" CACHE INTERNAL "")
set(MKLDNN_USE_MKL "NONE" CACHE INTERNAL "")
set(MKLDNN_ENABLE_CONCURRENT_EXEC ON CACHE INTERNAL "")
set(MKLDNN_USE_CLANG_SANITIZER "" CACHE INTERNAL "")
set(MKLDNN_VERBOSE OFF CACHE BOOL "")
set(MKLDNN_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(MKLDNN_BUILD_TESTS OFF CACHE INTERNAL "")
set(BENCHDNN_USE_RDPMC OFF CACHE INTERNAL "")

# Add modified version of MKL-DNN
add_subdirectory(mkl-dnn EXCLUDE_FROM_ALL)

# Include some modules from MKL-DNN
include(mkl-dnn/cmake/Threading.cmake)
include(mkl-dnn/cmake/TBB.cmake)
include(mkl-dnn/cmake/platform.cmake)
include(mkl-dnn/cmake/SDL.cmake)

# Propagate no warning flags
append(CMAKE_C_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")
append(CMAKE_CXX_FLAGS "${CMAKE_CCXX_NOWARN_FLAGS}")

## ----------------------------------------------------------------------------
## Open Image Denoise library
## ----------------------------------------------------------------------------

# Generate version.h
configure_file(
  "${PROJECT_SOURCE_DIR}/include/OpenImageDenoise/version.h.in"
  "${PROJECT_SOURCE_DIR}/include/OpenImageDenoise/version.h"
)

add_subdirectory(common EXCLUDE_FROM_ALL)

set(CORE_SOURCES
  include/OpenImageDenoise/oidn.h
  include/OpenImageDenoise/oidn.hpp
  include/OpenImageDenoise/version.h
  core/api.cpp
  core/common.h
  core/math.h
  core/device.h
  core/device.cpp
  core/buffer.h
  core/image.h
  core/filter.h
  core/filter.cpp
  core/node.h
  core/input_reorder.h
  core/output_reorder.h
  core/weights_reorder.h
  core/tone_mapping.h
  core/upsample.h
  core/network.h
  core/autoencoder.h
)

set(CORE_SOURCES_SSE41
  core/network.cpp
  core/autoencoder.cpp
  core/tone_mapping.cpp
)

include(resource)
generate_cpp_resources(WEIGHTS_SOURCES "oidn::weights"
  weights/rt_ldr.tza
  weights/rt_ldr_alb.tza
  weights/rt_ldr_alb_nrm.tza
  weights/rt_hdr.tza
  weights/rt_hdr_alb.tza
  weights/rt_hdr_alb_nrm.tza
)

set_source_files_properties(${CORE_SOURCES_SSE41} PROPERTIES COMPILE_FLAGS "${ISA_FLAGS_SSE41}")

add_library(${PROJECT_NAME} ${OIDN_LIB_TYPE} ${CORE_SOURCES} ${CORE_SOURCES_SSE41} ${WEIGHTS_SOURCES})

if(OIDN_STATIC_LIB)
  target_compile_definitions(${PROJECT_NAME} INTERFACE -DOIDN_STATIC_LIB)
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/mkl-dnn/include
    ${PROJECT_SOURCE_DIR}/mkl-dnn/src
    ${PROJECT_SOURCE_DIR}/mkl-dnn/src/common
    ${PROJECT_SOURCE_DIR}/mkl-dnn/src/cpu/xbyak
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    common mkldnn
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})
set_property(TARGET ${PROJECT_NAME} PROPERTY SOVERSION "0")

## ----------------------------------------------------------------------------
## Open Image Denoise examples
## ----------------------------------------------------------------------------

add_subdirectory(examples)

## ----------------------------------------------------------------------------
## Open Image Denoise install and packaging
## ----------------------------------------------------------------------------

include(install)

# Has to be last
include(CPack)
