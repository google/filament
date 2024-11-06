cmake_minimum_required(VERSION 3.19)
project(vkmemalloc)

set(TARGET vkmemalloc)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(PUBLIC_HDRS ${PUBLIC_HDR_DIR}/vk_mem_alloc.h)

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} INTERFACE)
target_sources(${TARGET} INTERFACE
    $<BUILD_INTERFACE:${PUBLIC_HDRS}>
    $<INSTALL_INTERFACE:include/vk_mem_alloc.h>
)
target_include_directories(${TARGET} INTERFACE
    $<BUILD_INTERFACE:${PUBLIC_HDR_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
