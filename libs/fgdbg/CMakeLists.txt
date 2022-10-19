cmake_minimum_required(VERSION 3.19)
project(fgdbg)

set(TARGET fgdbg)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(PUBLIC_HDRS include/fgdbg/DebugServer.h)

set(SRCS src/DebugServer.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TARGET} PUBLIC civetweb utils)
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
include_directories(${PUBLIC_HDR_DIR})
