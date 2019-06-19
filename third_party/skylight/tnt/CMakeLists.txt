cmake_minimum_required(VERSION 3.10)
project(skylight)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              skylight)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

# public headers dependencies
set(PUBLIC_HDRS
        ${PUBLIC_HDR_DIR}/ArHosekSkyModel.h
        )

# private headers dependencies
set(PRIVATE_HDRS
        ${PUBLIC_HDR_DIR}/ArHosekSkyModelData_CIEXYZ.h
        ${PUBLIC_HDR_DIR}/ArHosekSkyModelData_RGB.h
        ${PUBLIC_HDR_DIR}/ArHosekSkyModelData_Spectral.h
        )

# sources dependencies
set(SRCS
        ${SRC_DIR}/ArHosekSkyModel.c
        )

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
