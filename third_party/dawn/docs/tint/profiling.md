# Tint Profiling

Profiling is always something of an art form due to the interactions between the compiler and the machine code.
Here is some guidance
* Profile fully optimized builds. This includes LTO (use_thin_lto = true in args.gn).
* Fix clock frequencies where possible (usually easier on external devices like ChromeOS/Android)
* Determinism (this is probably a given for running the tint compiler directly)

See `benchmarking.md` for tint shader benchmarking setup and existing benchmarking shaders.

## Magic Trace

Magic trace is tracing profiler that uses x64 architecture instrumentation (Intel PT) to get full traces without manual or compiler instrumentation (https://github.com/janestreet/magic-trace). It can be built from source or one can use the prebuilt binary directly (assuming a trusted party).
(https://magic-trace.org/)


```
sudo ~/magic_trace/magic-trace run  -full-execution -multi-thread ./out/active/tint -- ./test/tint/benchmark/shadow-fragment.wgsl --format spirv
```

The resulting trace (default file name 'trace.fxt') can be viewed in perfetto (https://ui.perfetto.dev/)
Example trace:
(https://ui.perfetto.dev/#!/?s=9c2017442ae51f9e7dc6dcf041bce7003ca2095a)


Magic trace can sometimes produce misleading traces due to the tracing mechanism.
* Missing trace data due to trace size limits
* Unrealistic (and incorrect) timing in the initial phase of tracing
* Broken or spliced traces due to internal errors that do not reflect the actual call history
