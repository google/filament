cmake_minimum_required(VERSION 3.10)
project(swiftshader)

set(TARGET swiftshader)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET} INTERFACE ${PUBLIC_HDR_DIR})
