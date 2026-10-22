---
name: filament-desktop-testing
description: >
  Execute unit tests and performance benchmarks for Filament on desktop platforms.
  Use this skill to run and filter tests or benchmarks locally.
---

# Filament Desktop Testing and Benchmarking

This skill covers running tests and performance benchmarks on desktop platforms.

## 1. Executing a Unit Test

Run compiled unit test binaries directly from their respective build output directories.

### Example: Running utils unit tests (Debug)
```bash
./out/cmake-debug/libs/utils/test_utils
```

### Example: Running filament unit tests (Debug)
```bash
./out/cmake-debug/filament/test/test_filament
```

### Filtering Tests
Use standard Google Test (`gtest`) command-line parameters to run a subset of tests:
```bash
./out/cmake-debug/filament/test/test_filament --gtest_filter=Suite.TestName
```

---

## 2. Executing a Benchmark

Run compiled performance benchmarks from the release build output directories.

### Example: Running filament benchmarks (Release)
```bash
./out/cmake-release/filament/benchmark/benchmark_filament
```

### Filtering Benchmarks
Use standard Google Benchmark parameters to filter specific benchmarks:
```bash
./out/cmake-release/filament/benchmark/benchmark_filament --benchmark_filter=lutGeneration
```
