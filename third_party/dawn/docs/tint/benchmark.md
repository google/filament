# Tint Benchmark

The `tint_benchmark` executable measures the cost of translating shaders from WGSL to each backend language.
The benchmark uses [Google Benchmark](https://github.com/google/benchmark), and is integrated into [Chromium's performance waterfall](https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/speed/perf_waterfall.md).

## Benchmark inputs

Shaders used for benchmarking can be provided as either WGSL or SPIR-V.
The shaders are embedded in the binary at build time in order to avoid runtime dependencies.
A [Python script](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/tint/cmd/bench/generate_benchmark_inputs.py) is used to generate a header file that contains all of the benchmark shaders, and the macros that register benchmarks with the Google Benchmark harness.

SPIR-V shaders are converted to WGSL as an offline step using Tint, as the SPIR-V reader is not available on the waterfall bots.
The generated WGSL files are checked in to the repo and the same script is used by CQ to check that the generated files are up to date.

The script lists the paths to all of the benchmark shaders, which are either in the end-to-end test directory (`test/tint/benchmark/`) or provided as external shaders in `third_party/benchmark_shaders/`.

## Adding benchmarks

Files that end with `_bench.cc` are automatically included in the benchmark binary.
To benchmark a component of Tint against the benchmark shaders, define a function that performs the benchmarking and then register it with the `TINT_BENCHMARK_PROGRAMS` macro.
For example, the SPIR-V backend benchmark code is in [`src/tint/lang/spirv/writer/writer_bench.cc`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/tint/lang/spirv/writer/writer_bench.cc).

Other parts of Tint can be benchmarked independently from the benchmark shader corpus by registering the benchmark function with the `BENCHMARK` macro.

## Chromium performance waterfall

The `tint_benchmark` binary is a dependency of the `performance_test_suite_template_base` template in [chrome/test/BUILD.gn](https://source.chromium.org/chromium/chromium/src/+/refs/tags/131.0.6735.1:chrome/test/BUILD.gn;l=5909).

The [`bot_platforms.py`](https://source.chromium.org/chromium/chromium/src/+/main:tools/perf/core/bot_platforms.py) script controls which platforms run the benchmark.
The benchmark was added to the waterfall in [this CL](https://chromium-review.googlesource.com/c/chromium/src/+/5815585).

The [chrome.perf console](https://ci.chromium.org/p/chrome/g/chrome.perf/console) shows the status of the builder and tester bots, and can be used to check that changes to the benchmarking setup are working correctly.

## Viewing benchmark data

View benchmark data with the [perf.luci.app](https://perf.luci.app/) dashboard.
The units for Tint benchmarks are nanoseconds.

The query parameters are:

- `benchmark` - select `tint_benchmark`
- `bot` - select the waterfall bot from which to view data
- `test` - select the component of Tint to focus on (e.g. a specific backend)
- `subtest_1` - select the benchmark shader to focus on
