cmake_minimum_required(VERSION 3.10)
project(filament-benchmarks)

# ==================================================================================================
# Benchmarks
# ==================================================================================================

set(BENCHMARK_SRCS
        benchmark_filament.cpp)

add_executable(benchmark_filament ${BENCHMARK_SRCS})

if(HUNTER_ENABLED)
  target_link_libraries(benchmark_filament PRIVATE benchmark::benchmark benchmark::benchmark_main utils math filament)
else()
  target_link_libraries(benchmark_filament PRIVATE benchmark_main utils math filament)
endif()
