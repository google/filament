# How to run benchmarks

## Installing the executable on the device

`adb push out/cmake-android-release-aarch64/filament/benchmark/benchmark_filament /data/local/tmp`

## Running the benchmark

`adb shell /data/local/tmp/benchmark_filament --benchmark_counters_tabular=true`


## Benchmark results

### Macbook Pro M1 Pro
```
--------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations items_per_second
--------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           702 ns        702 ns     819874       729.274M/s
FilamentCullingFixture/sphereCulling        485 ns        485 ns    1430396       1054.82M/s
```

### Pixel 8 Pro
```
----------------------------------------------------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations        BPU          C        CPI          I items_per_second
----------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling          1212 ns       1208 ns     578797          0    6.87234   0.328354    20.9297       423.884M/s
FilamentCullingFixture/sphereCulling        748 ns        745 ns     938377          0    4.24185    0.39125    10.8418       686.839M/s
```

### Galaxy S20+
```
----------------------------------------------------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations        BPU          C        CPI          I items_per_second
----------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling          1695 ns       1688 ns     414849          0    9.35888   0.422963     22.127       303.397M/s
FilamentCullingFixture/sphereCulling       1160 ns       1147 ns     610602          0    6.35746   0.526617    12.0723       446.543M/s
```

### Pixel 4
```
----------------------------------------------------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations        BPU          C        CPI          I items_per_second
----------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling          2114 ns       2106 ns     332395          0    9.93665   0.449074     22.127       243.169M/s
FilamentCullingFixture/sphereCulling       1407 ns       1402 ns     497755          0    6.61423   0.547886    12.0723         365.3M/s
```

