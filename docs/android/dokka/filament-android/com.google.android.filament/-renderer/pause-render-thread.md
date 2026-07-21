//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[pauseRenderThread](pause-render-thread.md)

# pauseRenderThread

[main]\
open fun [pauseRenderThread](pause-render-thread.md)(durationNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Stalls the render thread (GPU submission pipeline) for the given duration in nanoseconds. 

This is useful for simulating long rendering frames (e.g. testing buffer stuffing recovery) without blocking the application's main event loop thread.

#### Parameters

main

| | |
|---|---|
| durationNanos | The duration to pause the render thread in nanoseconds. |
