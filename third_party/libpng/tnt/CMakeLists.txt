cmake_minimum_required(VERSION 3.19)
project(png)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              png)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

# public headers dependencies
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/png.h
    ${PUBLIC_HDR_DIR}/pngconf.h
    ${PUBLIC_HDR_DIR}/pnglibconf.h
)

# private headers dependencies
set(PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/pngdebug.h
    ${PRIVATE_HDR_DIR}/pnginfo.h
    ${PRIVATE_HDR_DIR}/pngpriv.h
    ${PRIVATE_HDR_DIR}/pngstruct.h
)

# sources dependencies
set(SRCS
    ${SRC_DIR}/png.c
    ${SRC_DIR}/pngerror.c
    ${SRC_DIR}/pngget.c
    ${SRC_DIR}/pngmem.c
    ${SRC_DIR}/pngpread.c
    ${SRC_DIR}/pngread.c
    ${SRC_DIR}/pngrio.c
    ${SRC_DIR}/pngrtran.c
    ${SRC_DIR}/pngrutil.c
    ${SRC_DIR}/pngset.c
    ${SRC_DIR}/pngtrans.c
    ${SRC_DIR}/pngwio.c
    ${SRC_DIR}/pngwrite.c
    ${SRC_DIR}/pngwtran.c
    ${SRC_DIR}/pngwutil.c
)

if (NOT LINUX AND NOT WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

# disable ARM neon optimizations, let the compiler do it if it can
target_compile_definitions(${TARGET} PRIVATE PNG_ARM_NEON_OPT=0)

target_link_libraries(${TARGET} PUBLIC z)

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
