# How to Run Filament Benchmarks

This document details how to build, deploy, configure, and execute benchmarks with hardware performance counters on connected Android devices.

---

## 1. Building the Benchmark

Compile the release binary for Android (`arm64-v8a`) with Perfetto tracing enabled:

```bash
# Build Android and desktop
./build.sh -q arm64-v8a -Pip desktop,android release

# Or build Android release only
./build.sh -q arm64-v8a -Pip android release
```

---

## 2. Installing on the Device

Push the compiled executable to `/data/local/tmp` using `adb`:

```bash
adb push out/cmake-android-release-aarch64/filament/benchmark/benchmark_filament /data/local/tmp/
```

---

## 3. Configuring Device Permissions for Hardware Performance Counters

Modern Android kernels restrict unprivileged user-space processes from accessing hardware Performance Monitoring Unit (PMU) counters via `perf_event_open(2)`.

If hardware performance counters (`C`, `I`, `BPU`, `CPI`) report `0` or `nan`, configure device permissions on a userdebug build:

```bash
adb root
adb shell setenforce 0
adb shell setprop security.perf_harden 0
```

---

## 4. Running the Benchmark

Run the pushed binary while setting `LD_LIBRARY_PATH` and specifying Google Benchmark options:

```bash
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/benchmark_filament \
    --benchmark_filter=FilamentCullingFixture \
    --benchmark_color=true \
    --benchmark_counters_tabular=true
```

---

## 5. CPU Affinity and `taskset`

Modern mobile SoCs use heterogeneous multi-core architectures (e.g. DynamIQ / big.LITTLE) with distinct core clusters:
* **Little Cores** (e.g. Cortex-A510 / A520): High efficiency, lower throughput.
* **Mid / Performance Cores** (e.g. Cortex-A715 / A720): Balanced performance and full PMU counter availability.
* **Prime / Big Core** (e.g. Cortex-X4): Maximum single-threaded performance.

### Why use `taskset`?
1. **Thread Migration Noise**: Without CPU affinity, the OS scheduler may migrate benchmark threads across different core types mid-run, creating noisy cycle counts and distorted timings.
2. **PMU Counter Constraints**: On some devices / hypervisors (e.g. pKVM), the Prime core may have general programmable event counters masked, while the Mid and Little core clusters expose multiple hardware counters for concurrent measurement of `CPU_CYCLES`, `INSTRUCTIONS`, `BRANCH_MISSES`, and cache metrics.

### Identifying Device CPU Cores (`print_cpu_topology.sh`)
To determine which CPU IDs correspond to which core clusters on your connected device, use the included helper script:

```bash
# Run from host (queries connected device over adb)
./filament/benchmark/print_cpu_topology.sh

# Or target a specific device serial
./filament/benchmark/print_cpu_topology.sh -s <device_serial>
```

Example output:
```text
CPU    Core Model       Max Clock    Capacity   Cluster     
-----  ---------------  -----------  ---------  ----------- 
0      Cortex-A510      1.70 GHz     182        Little      
1      Cortex-A510      1.70 GHz     182        Little      
2      Cortex-A510      1.70 GHz     182        Little      
3      Cortex-A510      1.70 GHz     182        Little      
4      Cortex-A715      2.37 GHz     725        Mid         
5      Cortex-A715      2.37 GHz     725        Mid         
6      Cortex-A715      2.37 GHz     725        Mid         
7      Cortex-A715      2.37 GHz     725        Mid         
8      Cortex-X3/X4     2.91 GHz     1024       Prime/Big   

Recommended taskset commands for benchmarking:
  - Mid Cluster   (Recommended): taskset -c 4-7
  - Little Cluster (Efficiency):  taskset -c 0-3
  - Prime/Big Core (Peak Speed):  taskset -c 8
```

> **Note on Columns**:
> * **Core Model**: Hardware microarchitecture identified from `/proc/cpuinfo` part numbers.
> * **Capacity**: Kernel compute capacity rating (`cpu_capacity` on a `0–1024` scale). Higher values reflect higher IPC and clock speed. Cores with capacity `1024` represent peak single-threaded performance.
> * **Cluster / Recommended taskset**: Automatically generates the optimal CPU core mask for your device.

### Pinning to Specific Clusters
Use `taskset -c <cpus>` to pin the benchmark:

* **Mid Cluster (Recommended for full PMU metrics)**:
  ```bash
  adb shell "taskset -c 4-7 /data/local/tmp/benchmark_filament --benchmark_filter=FilamentCullingFixture --benchmark_color=true --benchmark_counters_tabular=true"
  ```
* **Little Cluster**:
  ```bash
  adb shell "taskset -c 0-3 /data/local/tmp/benchmark_filament --benchmark_filter=FilamentCullingFixture --benchmark_color=true --benchmark_counters_tabular=true"
  ```
* **Prime / Big Core**:
  ```bash
  adb shell "taskset -c 8 /data/local/tmp/benchmark_filament --benchmark_filter=FilamentCullingFixture --benchmark_color=true --benchmark_counters_tabular=true"
  ```

---


## Benchmark results

### Macbook Pro M4 Max (08/2026)
```
--------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations items_per_second
--------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           367 ns        367 ns    1834809       1.39527G/s
FilamentCullingFixture/sphereCulling        257 ns        257 ns    2806185       1.99235G/s
```

### Macbook Pro M1 Pro
```
--------------------------------------------------------------------------------------
Benchmark                              Time           CPU Iterations items_per_second
--------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           702 ns        702 ns     819874       729.274M/s
FilamentCullingFixture/sphereCulling        485 ns        485 ns    1430396       1054.82M/s
```

### Pixel 10 Pro Fold (08/2026)
```
--------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations        BPU          C        CPI          I items_per_second
--------------------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           511 ns          511 ns      1358409   717.464n    3.77102   0.248841    15.1543       1.00273G/s
FilamentCullingFixture/sphereCulling        337 ns          337 ns      2077960   225.657u    2.48693   0.250898    9.91211       1.52031G/s
```

### Pixel 9 Pro XL (08/2026)
```
--------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations        BPU          C        CPI          I items_per_second
--------------------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           623 ns          622 ns      1062119          0          0        nan          0       823.182M/s
FilamentCullingFixture/sphereCulling        413 ns          412 ns      1708340          0          0        nan          0       1.24151G/s
```

### Pixel 8 Pro (08/2026)
```
--------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                     Time             CPU   Iterations        BPU          C        CPI          I items_per_second
--------------------------------------------------------------------------------------------------------------------------------------------
FilamentCullingFixture/boxCulling           707 ns          703 ns       929075          0    3.91987   0.258664    15.1543       728.759M/s
FilamentCullingFixture/sphereCulling        484 ns          480 ns      1479801          0    2.61268   0.263585    9.91211       1.06603G/s
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

