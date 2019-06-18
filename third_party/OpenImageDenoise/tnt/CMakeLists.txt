cmake_minimum_required(VERSION 3.10)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

configure_file(
  "${PROJECT_SOURCE_DIR}/../include/OpenImageDenoise/version.h.in"
  "${PROJECT_SOURCE_DIR}/../include/OpenImageDenoise/version.h"
)

set(MKLDNN_LIBRARY_TYPE "STATIC" CACHE INTERNAL "")
set(MKLDNN_THREADING "TBB" CACHE INTERNAL "")
set(MKLDNN_USE_MKL "NONE" CACHE INTERNAL "")
set(MKLDNN_ENABLE_CONCURRENT_EXEC ON CACHE INTERNAL "")
set(MKLDNN_USE_CLANG_SANITIZER "" CACHE INTERNAL "")
set(MKLDNN_VERBOSE OFF CACHE BOOL "")
set(MKLDNN_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(MKLDNN_BUILD_TESTS OFF CACHE INTERNAL "")
set(BENCHDNN_USE_RDPMC OFF CACHE INTERNAL "")

add_subdirectory(../mkl-dnn/tnt "${CMAKE_CURRENT_BINARY_DIR}/mkl-dnn")

set(CORE_SOURCES
  ../include/OpenImageDenoise/oidn.h
  ../include/OpenImageDenoise/oidn.hpp
  ../include/OpenImageDenoise/version.h
  ../core/api.cpp
  ../core/common.h
  ../core/math.h
  ../core/device.h
  ../core/device.cpp
  ../core/buffer.h
  ../core/image.h
  ../core/filter.h
  ../core/filter.cpp
  ../core/node.h
  ../core/input_reorder.h
  ../core/output_reorder.h
  ../core/weights_reorder.h
  ../core/tone_mapping.h
  ../core/upsample.h
  ../core/network.h
  ../core/autoencoder.h
)

set(COMMON_SOURCES
  ../common/platform.h
  ../common/platform.cpp
  ../common/exception.h
  ../common/ref.h
  ../common/barrier.h
  ../common/timer.h
  ../common/thread.h
  ../common/thread.cpp
  ../common/tasking.h
  ../common/tasking.cpp
  ../common/tensor.h
  ../common/tensor.cpp
)

set(CORE_SOURCES_SSE41
  ../core/network.cpp
  ../core/autoencoder.cpp
  ../core/tone_mapping.cpp
)

generate_cpp_resources(WEIGHTS_SOURCES "oidn::weights"
  ../weights/rt_ldr.tza
  ../weights/rt_ldr_alb.tza
  ../weights/rt_ldr_alb_nrm.tza
  ../weights/rt_hdr.tza
  ../weights/rt_hdr_alb.tza
  ../weights/rt_hdr_alb_nrm.tza
)

add_library(${PROJECT_NAME} ${OIDN_LIB_TYPE}
    ${CORE_SOURCES}
    ${COMMON_SOURCES}
    ${CORE_SOURCES_SSE41}
    ${WEIGHTS_SOURCES})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/../mkl-dnn/include
    ${PROJECT_SOURCE_DIR}/../mkl-dnn/src
    ${PROJECT_SOURCE_DIR}/../mkl-dnn/src/common
    ${PROJECT_SOURCE_DIR}/../mkl-dnn/src/cpu/xbyak
)

target_link_libraries(${PROJECT_NAME} PRIVATE mkldnn)
