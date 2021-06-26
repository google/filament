# How to run benchmarks

## Installing the executable on the device

`adb push out/cmake-android-release-aarch64/filament/benchmark/benchmark_filament /data/local/tmp`

## Running the benchmark

`adb shell /data/local/tmp/benchmark_filament`


## Benchmark results

### Galaxy S20+
```
----------------------------------------------------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations        BPU          C        CPI          I items_per_second
----------------------------------------------------------------------------------------------------------------------------------
FilamentFixture/boxCulling          1695 ns       1688 ns     414849          0    9.35888   0.422963     22.127       303.397M/s
FilamentFixture/sphereCulling       1160 ns       1147 ns     610602          0    6.35746   0.526617    12.0723       446.543M/s
```

### Pixel 4
```
----------------------------------------------------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations        BPU          C        CPI          I items_per_second
----------------------------------------------------------------------------------------------------------------------------------
FilamentFixture/boxCulling          2114 ns       2106 ns     332395          0    9.93665   0.449074     22.127       243.169M/s
FilamentFixture/sphereCulling       1407 ns       1402 ns     497755          0    6.61423   0.547886    12.0723         365.3M/s
```
