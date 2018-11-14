cmake_minimum_required(VERSION 3.1)
project(smol-v)

set(TARGET smol-v)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)

set(SRCS
    ${SRC_DIR}/smolv.cpp
)

add_library(${TARGET} STATIC ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
