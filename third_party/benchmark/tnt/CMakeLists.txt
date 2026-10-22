option(FILAMENT_USE_EXTERNAL_BENCHMARK "Use find_package to locate an external benchmark dependency" OFF)

if (FILAMENT_USE_EXTERNAL_BENCHMARK)
  find_package(benchmark CONFIG QUIET)
  return()
endif()

cmake_minimum_required(VERSION 3.19)
project(benchmark)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              benchmark)
set(SRC_DIR             ${OUR_DIR}/src)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/include)
set(PRIVATE_HDR_DIR     ${OUR_DIR})

# sources dependencies
file(GLOB
        SRCS
        ${SRC_DIR}/*.cc
        ${PUBLIC_HDR_DIR}/benchmark/*.h
        ${PRIVATE_HDR_DIR}/*.h)
list(REMOVE_ITEM SRCS "${SRC_DIR}/benchmark_main.cc")

include_directories(
        ${PUBLIC_HDR_DIR}
        ${PRIVATE_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${SRCS})

set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET} PROPERTIES CXX_STANDARD_REQUIRED YES)
set_target_properties(${TARGET} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_definitions(${TARGET} PUBLIC BENCHMARK_STATIC_DEFINE)

if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE
            -Wno-deprecated-declarations
            -Wno-unused-variable)
endif()

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#include <pthread.h>
int main() {
  cpu_set_t set;
  CPU_ZERO(&set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
  return 0;
}
" HAVE_PTHREAD_AFFINITY)

if(HAVE_PTHREAD_AFFINITY)
  target_compile_definitions(${TARGET} PRIVATE BENCHMARK_HAS_PTHREAD_AFFINITY)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${TARGET} PRIVATE Threads::Threads)

if (LINUX)
  target_link_libraries(${TARGET} PRIVATE rt)
endif()

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE shlwapi)
endif()

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)

# Benchmark main library
add_library(${TARGET}_main ${SRC_DIR}/benchmark_main.cc)
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
target_link_libraries(${TARGET}_main benchmark)
set_target_properties(${TARGET}_main PROPERTIES FOLDER ThirdParty)
