cmake_minimum_required(VERSION 3.10)
project(math)

set(TARGET math)
set(PUBLIC_HDR_DIR include)

if (MSVC OR CLANG_CL)
    set(OPTIMIZATION_FLAGS /fp:fast)
else()
    set(OPTIMIZATION_FLAGS -ffast-math -ffp-contract=fast)
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(SRCS dummy.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_compile_options(${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Installation
# ==================================================================================================
install(DIRECTORY ${PUBLIC_HDR_DIR}/math DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
add_executable(test_${TARGET}
        tests/test_fast.cpp
        tests/test_half.cpp
        tests/test_mat.cpp
        tests/test_vec.cpp
        tests/test_quat.cpp
)
target_link_libraries(test_${TARGET} PRIVATE math gtest)

# ==================================================================================================
# Benchmarks
# ==================================================================================================

set(BENCHMARK_SRCS
        benchmarks/benchmark_fast.cpp)

add_executable(benchmark_${TARGET} ${BENCHMARK_SRCS})

target_compile_options(benchmark_${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})

target_link_libraries(benchmark_${TARGET} PRIVATE benchmark_main utils math)
