cmake_minimum_required(VERSION 3.10)
project(civetweb)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              civetweb)

set(SRC_DIR             ${OUR_DIR}/src)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/include)
set(PRIVATE_HDR_DIR     ${OUR_DIR})

set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/CivetServer.h
    ${PUBLIC_HDR_DIR}/civetweb.h
)

set(PRIVATE_HDRS
    # None
)

set(SRCS
    ${SRC_DIR}/civetweb.c
    ${SRC_DIR}/CivetServer.cpp
)

add_definitions(-DUSE_WEBSOCKET)

add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

target_link_libraries(${TARGET} ${CMAKE_DL_LIBS})

target_compile_options(${TARGET} PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)

install(TARGETS ${TARGET} ARCHIVE DESTINATION third_party/${DIST_DIR})
