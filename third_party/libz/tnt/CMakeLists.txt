cmake_minimum_required(VERSION 3.19)
project(z)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              z)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

# public headers dependencies
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/zconf.h
    ${PUBLIC_HDR_DIR}/zlib.h
)

# private headers dependencies
set(PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/crc32.h
    ${PRIVATE_HDR_DIR}/deflate.h
    ${PRIVATE_HDR_DIR}/gzguts.h
    ${PRIVATE_HDR_DIR}/inffast.h
    ${PRIVATE_HDR_DIR}/inffixed.h
    ${PRIVATE_HDR_DIR}/inflate.h
    ${PRIVATE_HDR_DIR}/inftrees.h
    ${PRIVATE_HDR_DIR}/trees.h
    ${PRIVATE_HDR_DIR}/zutil.h
)

# sources dependencies
set(SRCS
    ${SRC_DIR}/adler32.c
    ${SRC_DIR}/compress.c
    ${SRC_DIR}/crc32.c
    ${SRC_DIR}/deflate.c
    ${SRC_DIR}/gzclose.c
    ${SRC_DIR}/gzlib.c
    ${SRC_DIR}/gzread.c
    ${SRC_DIR}/gzwrite.c
    ${SRC_DIR}/inflate.c
    ${SRC_DIR}/infback.c
    ${SRC_DIR}/inftrees.c
    ${SRC_DIR}/inffast.c
    ${SRC_DIR}/trees.c
    ${SRC_DIR}/uncompr.c
    ${SRC_DIR}/zutil.c
)

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

if (NOT MSVC)
    target_compile_options(${TARGET}
        PRIVATE -Wno-implicit-function-declaration
        PRIVATE -Wno-shift-negative-value
    )
endif()

# specify where the public headers of this library are
target_include_directories (${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
