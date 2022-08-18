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
file(GLOB BENCHMARK_MAIN "benchmark_main.cc")
foreach(item ${BENCHMARK_MAIN})
  list(REMOVE_ITEM SOURCE_FILES "${item}")
endforeach()

if (NOT MSVC)
  add_definitions(
          -Wno-deprecated-declarations
  )
endif()

include_directories(
        ${PUBLIC_HDR_DIR}
        ${PRIVATE_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${SRCS})

if (LINUX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} Threads::Threads)
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
