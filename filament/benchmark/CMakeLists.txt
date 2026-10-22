cmake_minimum_required(VERSION 3.19)
project(filament-benchmarks)

# FIXME: this is the same logic as in filament's test. without it we have a build failure on GL calls.
if(IOS OR WASM)
    return()
endif()

# ==================================================================================================
# Benchmarks
# ==================================================================================================

set(BENCHMARK_SRCS
        benchmark_BufferAllocator.cpp
        benchmark_filament.cpp)

add_executable(benchmark_filament ${BENCHMARK_SRCS})

target_link_libraries(benchmark_filament PRIVATE benchmark_main utils filament)
target_compile_options(benchmark_filament PRIVATE ${COMPILER_FLAGS})

set_target_properties(benchmark_filament PROPERTIES FOLDER Benchmarks)
