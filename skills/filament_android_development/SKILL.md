---
name: filament-android-development
description: >
  Build, deploy, run, and benchmark Filament binaries on connected Android devices or emulators.
  Use this skill for compiling, pushing, and executing Android tests or benchmarks.
---

# Filament Android Development, Deployment, and Benchmarking

This skill details the procedures for compiling binaries for Android, installing them onto a physical device or emulator, and running tests and benchmarks.

## 1. Compiling Release Build for Android

Android binaries should almost always be built in release mode for testing and benchmarking. 
To build the release version targeting the `arm64-v8a` architecture, and optionally enable **Perfetto** tracing (the `-P` flag, which is highly recommended during performance development):

```bash
./build.sh -q arm64-v8a -Pip android release
```

---

## 2. Deploying Binaries to an Android Device

Push the compiled executable (test or benchmark) to a writable temporary directory (such as `/data/local/tmp`) on the connected Android device using `adb`:

```bash
adb push ./out/cmake-android-release-aarch64/{path-to-executable} /data/local/tmp
```

*Replace `{path-to-executable}` with the actual relative path to your compiled binary under `out/cmake-android-release-aarch64/`.*

---

## 3. Executing Binaries on an Android Device

Run the pushed binary inside the Android device's shell. 
You must set `LD_LIBRARY_PATH` to the directory containing the pushed binary so that the dynamic linker can locate target proprietary shared libraries:

```bash
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/{executable}
```

---

## 4. Running a Performance Benchmark on Device

When running performance benchmarks on Android devices, always conform to the following formatting rules:
1. Output counters as a table (`--benchmark_counters_tabular`)
2. Keep colors enabled (`--benchmark_color=true`)
3. Set an appropriate, readable time unit matching the performance characteristics of the measured workload (typically `ms` or `us`, rarely `ns`):

```bash
adb shell LD_LIBRARY_PATH=/data/local/tmp /data/local/tmp/{executable} --benchmark_counters_tabular --benchmark_color=true --benchmark_time_unit=ms
```
