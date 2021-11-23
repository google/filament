cmake_minimum_required(VERSION 3.19)
project(trie)

set(TARGET trie)
set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        ${OUR_DIR}/tsl/htrie_hash.h
        ${OUR_DIR}/tsl/htrie_map.h
        ${OUR_DIR}/tsl/htrie_set.h
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${OUR_DIR}/tsl)

add_library(${TARGET} INTERFACE)
target_sources(${TARGET} INTERFACE ${PUBLIC_HDRS})
target_include_directories(${TARGET} INTERFACE ${OUR_DIR})
