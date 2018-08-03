cmake_minimum_required(VERSION 3.1)
project(stb)

set(TARGET stb)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(PUBLIC_HDR_DIR ${OUR_DIR})

file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/*.h)
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} INTERFACE)
target_sources(${TARGET} INTERFACE ${PUBLIC_HDRS})
target_include_directories(${TARGET} INTERFACE ${PUBLIC_HDR_DIR})
