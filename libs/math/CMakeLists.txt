cmake_minimum_required(VERSION 3.19)
project(math)

set(TARGET math)
set(PUBLIC_HDR_DIR include)

if (MSVC)
    set(OPTIMIZATION_FLAGS /fp:fast)
else()
    set(OPTIMIZATION_FLAGS -ffast-math -fno-finite-math-only -ffp-contract=fast)
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
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Installation
# ==================================================================================================
install(DIRECTORY ${PUBLIC_HDR_DIR}/math DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if(FILAMENT_TESTS)
add_executable(test_${TARGET}
        tests/test_fast.cpp
        tests/test_half.cpp
        tests/test_mat.cpp
        tests/test_vec.cpp
        tests/test_quat.cpp
)
target_link_libraries(test_${TARGET} PRIVATE math gtest)
set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()

# ==================================================================================================
# Benchmarks
# ==================================================================================================
if(FILAMENT_BENCHMARKS)
set(BENCHMARK_SRCS
        benchmarks/benchmark_fast.cpp include/math/mathfwd.h)

add_executable(benchmark_${TARGET} ${BENCHMARK_SRCS})

target_compile_options(benchmark_${TARGET} PRIVATE ${OPTIMIZATION_FLAGS})

target_link_libraries(benchmark_${TARGET} PRIVATE benchmark_main utils math)

set_target_properties(benchmark_${TARGET} PROPERTIES FOLDER Benchmarks)
endif()
