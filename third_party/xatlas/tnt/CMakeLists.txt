cmake_minimum_required(VERSION 3.10)
project(xatlas)

set(TARGET xatlas)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(PUBLIC_HDRS ${PUBLIC_HDR_DIR}/xatlas.h)

set(SRCS ${SRC_DIR}/xatlas.cpp)

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories (${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
