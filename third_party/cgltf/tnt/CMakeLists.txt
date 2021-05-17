cmake_minimum_required(VERSION 3.19)
project(cgltf)

set(TARGET cgltf)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/cgltf_write.h
    ${PUBLIC_HDR_DIR}/cgltf.h)

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} INTERFACE)
target_sources(${TARGET} INTERFACE ${PUBLIC_HDRS})
target_include_directories(${TARGET} INTERFACE ${PUBLIC_HDR_DIR})
