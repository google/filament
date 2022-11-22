cmake_minimum_required(VERSION 3.19)
project(stb)

set(TARGET stb)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(PUBLIC_HDRS ../stb_image.h)

set(SRCS ../stb_image.cpp)

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)

install(TARGETS stb ARCHIVE DESTINATION lib/${DIST_DIR})
