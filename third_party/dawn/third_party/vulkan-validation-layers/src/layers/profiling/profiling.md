# Profiling VVL

The [Tracy](https://github.com/wolfpld/tracy) profiler has been setup. Get the doc [here](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf).

Tested with Tracy version 0.11.1

Relevant CMake options:
- `-D VVL_ENABLE_TRACY` Enable Tracy
- `-D VVL_ENABLE_TRACY_CPU_MEMORY` Enable Tracy CPU memory profiling
- `-D VVL_TRACY_CALLSTACK=<N>` Define maximum collected call stack size (default: 48)
- `-D VVL_ENABLE_TRACY_GPU` Enable GPU profiling: (only retrieves timings for draw, dispatch and trace rays commands)

⚠️ Having a big call stack size can have a noticeable impact on performance

⚠️ Make sure your the various dependencies are compiled with the same optimisations levels and debug as VVL, otherwise expect crashes in `TracyAlloc/Free`

- To enable retrieving data from kernel facilities, for instance to have fine grained info on CPU usage by performing sampling, run the application VVL is injected into with elevated privileges. If you use VkConfig to enable VVL, do not forget to also launch it with elevated privileges.
⚠️ On windows, having other application running with elevated privileges can cause Tracy sampling to fail.

## Limitations

- You may notice a stall when shutting down the profiled application: have a look at the profiler, the "query backlog" (satellite icon, around top right) is probably being emptied. It can take some time.

- Meant to be used with applications that do not live for only a small amount of time, and create only one `VkInstance`.
=> So for now, forget about profiling our test suite...

- Manual profiler lifetimes are used: profiler is started when the VVL shared library is loaded, and it is shut down at `vkDestoyInstance` time.
=> It is thus assumed that application only create one instance, as of writing it appears to be the case in most applications.

- CPU memory profiling cannot be used with Mimalloc. It needs to be setup, a quick stab at it showed that it is blowing up Tracy.

## GPU profiling 

- Implementation implicitly relies on the following Vulkan features:
VK_EXT_calibrated_timestamps
VK_EXT_host_query_reset
vkGetPhysicalDeviceCalibrateableTimeDomainsEXT returning VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT on Windows and VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER on Unix

Those features seem to be present on all drivers used by the main VVL developpers.

⚠️ Required features are added by hacking into `ValidationStateTracker::PreCallRecordCreateDevice`, so profiling can only work if `ValidationStateTracker` is somehow instantiated.

When looking at the `Statistics > GPU` window of a trace, you may noticed that some action commands are missing compared to the amount of action commands submitted by the host.
It just means that by the time application shuts down, not all GPU time stamp queries have been retrieved.
