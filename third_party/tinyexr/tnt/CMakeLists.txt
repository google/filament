cmake_minimum_required(VERSION 3.19)
project(tinyexr)

set(TARGET tinyexr)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PUBLIC_HDR_DIR ${SRC_DIR})

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS ${SRC_DIR}/tinyexr.h)

set(SRCS ${SRC_DIR}/tnt/tinyexr.cc)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories (${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_link_libraries(${TARGET} LINK_PUBLIC z)
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)