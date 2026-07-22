//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[setupFrame](setup-frame.md)

# setupFrame

[main]\
open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

Prepares and evaluates the frame pacing state for the upcoming frame cycle. 

This must be called at the very beginning of the host display platform's VSYNC interrupt loop (e.g., within Android's `Choreographer` callback). It evaluates whether the CPU rendering thread is ahead of the hardware pulse cadence or backlogged.

#### Return

FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Incoming hardware base VSYNC timestamp in nanoseconds. |
| vsyncPeriodNanos | Physical display VSYNC refresh period in nanoseconds. |

[main]\
open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

Prepares and evaluates the frame pacing state for the upcoming frame cycle. 

This must be called at the very beginning of the host display platform's VSYNC interrupt loop (e.g., within Android's `Choreographer` callback). It evaluates whether the CPU rendering thread is ahead of the hardware pulse cadence or backlogged.

#### Return

FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Incoming hardware base VSYNC timestamp in nanoseconds. |

[main]\
open fun [setupFrame](setup-frame.md)(frameData: FrameData, vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

Prepares and evaluates the frame pacing state for the upcoming frame cycle using Android 13+ FrameData. 

This must be called at the very beginning of the host display platform's VSYNC interrupt loop (e.g., within Android's `Choreographer` callback). It evaluates whether the CPU rendering thread is ahead of the hardware pulse cadence or backlogged.

#### Return

FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.

#### Parameters

main

| | |
|---|---|
| frameData | Native VSYNC telemetry object received in an Android 13+ Choreographer.VsyncCallback. |
| vsyncPeriodNanos | Physical display VSYNC refresh period in nanoseconds. |
