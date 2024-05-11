cmake_minimum_required(VERSION 3.19)
project(filament-benchmarks)

# ==================================================================================================
# Benchmarks
# ==================================================================================================

set(BENCHMARK_SRCS
        benchmark_filament.cpp)

add_executable(benchmark_filament ${BENCHMARK_SRCS})

target_link_libraries(benchmark_filament PRIVATE benchmark_main filament)

set_target_properties(benchmark_filament PROPERTIES FOLDER Benchmarks)
