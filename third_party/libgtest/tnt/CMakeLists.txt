cmake_minimum_required(VERSION 3.19)
project(gtest)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              gtest)
set(TEST_SRC_DIR        ${OUR_DIR}/googletest/src)
set(MOCK_SRC_DIR        ${OUR_DIR}/googlemock/src)
set(PUBLIC_HDR_DIRS     ${OUR_DIR}/googletest/include ${OUR_DIR}/googlemock/include)
set(PRIVATE_HDR_DIRS    ${OUR_DIR}/googletest ${OUR_DIR}/googlemock)

# sources dependencies
set(SRCS
    ${TEST_SRC_DIR}/gtest-all.cc
    ${MOCK_SRC_DIR}/gmock-all.cc
)

if (MSVC)
  # The /Zc:__cplusplus compiler option enables the __cplusplus preprocessor macro to
  # report an updated value for recent C++ language standards support. By default,
  # Visual Studio always returns the value "199711L" for the __cplusplus preprocessor macro.
  add_definitions(/Zc:__cplusplus)
endif()

include_directories(
    ${PUBLIC_HDR_DIRS}
    ${PRIVATE_HDR_DIRS})

# we're building a library
add_library(${TARGET} STATIC ${SRCS})

if (LINUX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET} Threads::Threads)
endif()

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIRS})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)
