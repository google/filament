//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)/[setupFrame](setup-frame.md)

# setupFrame

[main]\
open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), hardwareTimelines: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)&gt;, timelineCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

[main]\
open fun [setupFrame](setup-frame.md)(tick: [FramePacer.VsyncTick](-vsync-tick/index.md)): [FramePacer.FrameStatus](-frame-status/index.md)

Prepares and evaluates the frame pacing state for the upcoming frame cycle. 

This must be called at the very beginning of the host display platform's VSYNC interrupt loop (e.g., within a VSYNC callback such as AChoreographer on Android). It evaluates whether the CPU rendering thread is ahead of the hardware pulse cadence or backlogged.

@attention The memory backed by the `tick.timelines` slice is guaranteed to be valid only during the immediate, synchronous execution of this call. Client implementations must not store or access this slice or its underlying pointers once `setupFrame()` returns.

#### Return

FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.

#### Parameters

main

| | |
|---|---|
| tick | Polled hardware VSYNC timing telemetry and optional target presentation timelines. |
