cmake_minimum_required(VERSION 3.19)
project(jsmn)

set(TARGET jsmn)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(PUBLIC_HDRS ${PUBLIC_HDR_DIR}/jsmn.h)

include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} INTERFACE)
target_sources(${TARGET} INTERFACE ${PUBLIC_HDRS})
target_include_directories(${TARGET} INTERFACE ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
