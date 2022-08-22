cmake_minimum_required(VERSION 3.19)
project(gtest)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              gtest)
set(SRC_DIR             ${OUR_DIR}/googletest/src)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/googletest/include)
set(PRIVATE_HDR_DIR     ${OUR_DIR}/googletest)

# sources dependencies
set(SRCS
    ${SRC_DIR}/gtest-all.cc
)

if (MSVC)
  # The /Zc:__cplusplus compiler option enables the __cplusplus preprocessor macro to
  # report an updated value for recent C++ language standards support. By default,
  # Visual Studio always returns the value "199711L" for the __cplusplus preprocessor macro.
  add_definitions(/Zc:__cplusplus)
endif()

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
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
