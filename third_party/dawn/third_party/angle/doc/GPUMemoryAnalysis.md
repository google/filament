# GPU Memory Reporting and Analysis

[MemRptExt]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_device_memory_report.html
[enabling-general-logging]: DebuggingTips.md#enabling-general-logging

GPU memory usage data can be reported when using the Vulkan back-end with drivers that support the
[VK_EXT_device_memory_report][MemRptExt] extension.  When enabled, ANGLE will produce log messages
based on every allocation, free, import, unimport, and failed allocation of GPU memory.  This
functionality requires [enabling general logging](#enabling-general-logging) as well as enabling
one or two feature flags.

## GPU Memory Reporting

ANGLE registers a callback function with the Vulkan driver for the
[VK_EXT_device_memory_report][MemRptExt] extension.  The Vulkan driver calls this callback for
each of the following GPU memory events:

- Allocation of GPU memory by ANGLE
- Free of GPU memory by ANGLE
- Import of GPU memory provided by another process (e.g. Android SurfaceFlinger)
- Unimport of GPU memory provided by another process
- Failed allocation

The callback provides additional information about each event such as the size, the VkObjectType,
and the address (see the extension documentation for more details).  ANGLE caches this information,
and logs messages based on this information.  ANGLE keeps track of how much of each type of memory
is allocated and imported.  For example, if a GLES command causes ANGLE five 4 KB descriptor set
(VK_OBJECT_TYPE_DESCRIPTOR_SET) allocations, ANGLE will add 20 KB to the total of allocated
descriptor set memory.

ANGLE supports two types of memory reporting, both of which are enabled
via feature flags:

* `logMemoryReportStats` provides summary statistics at each eglSwapBuffers() command
* `logMemoryReportCallbacks` provides per-callback information at the time of the callback

Both feature flags can be enabled at the same time.  A simple way to enable either or both of these
feature flags on Android is with with the following command:
```
adb shell setprop debug.angle.feature_overrides_enabled <feature>[:<feature>]
```
where `<feature>` is either `logMemoryReportStats` or `logMemoryReportCallbacks`.  Both can be
enabled by putting a colon between them, such as the following:
```
adb shell setprop debug.angle.feature_overrides_enabled logMemoryReportCallbacks:logMemoryReportStats
```

Another way to enable either or both of these feature flags is by editing the `vk_renderer.cpp` file,
and changing `false` in the following lines to `true`:
```
    ANGLE_FEATURE_CONDITION(&mFeatures, logMemoryReportCallbacks, false);
    ANGLE_FEATURE_CONDITION(&mFeatures, logMemoryReportStats, false);
```

Note: At this time, GPU memory reporting has only been tested and used on Android, where the logged
information can be viewed with the `adb logcat` command.

## GPU Memory Analysis

GPU memory reporting can be combined with other forms of debugging in order to do analysis.  For
example, for a GLES application/test that properly shuts down, the total size of each type of
allocated and imported memory should be zero bytes at the end of the application/test.  If not, a
memory leak exists, and the log can be used to determine where the leak occurs.

If an application seems to be using too much GPU memory, enabling memory reporting can reveal which
type of memory is being excessively used.

Complex forms of analysis can be done by enabling logging of every GLES and EGL API command.  This
can be enabled at compilation time by [enabling general logging](#enabling-general-logging) as well
as setting the following GN arg:
```
angle_enable_trace_android_logcat = true
```

Combining that with enabling the `logMemoryReportCallbacks` feature flag will allow each memory
allocation and import to be correlated with the GLES/EGL commands that caused it.  If more context
is needed for the type of drawing and/or setup that is being done in a sea of GLES commands, this
can also be combined with the use of a graphics debugger such as Android GPU Inspector (AGI) or
RenderDoc.  The debugger can help you understand what the application is doing at the time of the
particular GPU memory event is occuring.  For example, you might determine that the application is
doing something to cause a memory leak; or you may get insight into what the game is doing that
contributes to ANGLE using excessive amounts of GPU memory.
