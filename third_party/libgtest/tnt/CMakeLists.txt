cmake_minimum_required(VERSION 3.10)
project(gtest)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              gtest)
set(SRC_DIR             ${OUR_DIR}/src)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/include)
set(PRIVATE_HDR_DIR     ${OUR_DIR})

# sources dependencies
set(SRCS
    ${SRC_DIR}/gtest-all.cc
)


include_directories(
    ${PUBLIC_HDR_DIR}
    ${PRIVATE_HDR_DIR})

# we're building a library
add_library(${TARGET} STATIC ${SRCS})

if (LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} Threads::Threads)
endif()

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
