//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setRenderingDeadline](set-rendering-deadline.md)

# setRenderingDeadline

[main]\
open fun [setRenderingDeadline](set-rendering-deadline.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Set the deadline timestamp on the steady clock by which CPU and GPU rendering must complete for the buffer to meet its target display latching window. This must be called before [endFrame](end-frame.md).

#### Parameters

main

| | |
|---|---|
| monotonicClockNanos | The deadline timestamp on the steady clock in nanoseconds. |
