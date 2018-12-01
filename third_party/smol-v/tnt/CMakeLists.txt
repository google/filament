cmake_minimum_required(VERSION 3.1)
project(smol-v)

set(TARGET smol-v)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)

set(SRCS
    ${SRC_DIR}/smolv.cpp
)

add_library(${TARGET} STATIC ${SRCS})

target_compile_options(${TARGET} PRIVATE
        -Wno-tautological-unsigned-enum-zero-compare
        $<$<PLATFORM_ID:Linux>:-fPIC>
)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
