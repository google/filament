//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[setupFrame](setup-frame.md)

# setupFrame

[main]\
open fun [setupFrame](setup-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriodNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [FramePacer.FrameStatus](-frame-status/index.md)

Prepares and evaluates the frame pacing state for the upcoming frame cycle.

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

#### Return

FrameStatus::ACCEPTED if approved, or the specific SKIPPED reason.

#### Parameters

main

| | |
|---|---|
| frameData | Native VSYNC telemetry object received in an Android 13+ Choreographer.VsyncCallback. |
| vsyncPeriodNanos | Physical display VSYNC refresh period in nanoseconds. |
