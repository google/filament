cmake_minimum_required(VERSION 3.10)
project(math)

set(TARGET math)
set(PUBLIC_HDR_DIR include)

if (MSVC)
    set(OPTIMIZATION_FLAGS /fp:fast)
else()
    set(OPTIMIZATION_FLAGS -ffast-math -ffp-contract=fast)
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/math/TMatHelpers.h
        include/math/TQuatHelpers.h
        include/math/TVecHelpers.h
        include/math/compiler.h
        include/math/fast.h
        include/math/half.h
        include/math/mathfwd.h
        include/math/mat2.h
        include/math/mat3.h
        include/math/mat4.h
        include/math/norm.h
        include/math/quat.h
        include/math/scalar.h
        include/math/vec2.h
        include/math/vec3.h
        include/math/vec4.h
)

set(SRCS src/dummy.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_compile_options(${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})

if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME})  
endif()

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

if(HUNTER_ENABLED)
  target_link_libraries(test_${TARGET} PRIVATE math GTest::gtest GTest::gtest_main Threads::Threads)
else()
  target_link_libraries(test_${TARGET} PRIVATE math gtest)
endif()

# ==================================================================================================
# Benchmarks
# ==================================================================================================

set(BENCHMARK_SRCS
        benchmarks/benchmark_fast.cpp include/math/mathfwd.h)

add_executable(benchmark_${TARGET} ${BENCHMARK_SRCS})

target_compile_options(benchmark_${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})

if(HUNTER_ENABLED)
  target_link_libraries(benchmark_${TARGET} PRIVATE benchmark::benchmark benchmark::benchmark_main utils math)
else()
  target_link_libraries(benchmark_${TARGET} PRIVATE benchmark_main utils math)
endif()
