//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setRenderingDeadline](set-rendering-deadline.md)

# setRenderingDeadline

[main]\
open fun [setRenderingDeadline](set-rendering-deadline.md)(monotonic_clock_ns: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Set the deadline time point on the steady clock by which CPU and GPU rendering must complete for the buffer to meet its target display latching window. 

This must be called before endFrame().

#### Parameters

main

| | |
|---|---|
| monotonic_clock_ns | the deadline timestamp in nanoseconds on the steady clock. |
