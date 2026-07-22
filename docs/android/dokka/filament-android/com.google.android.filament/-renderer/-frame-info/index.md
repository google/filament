//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[FrameInfo](index.md)

# FrameInfo

open class [FrameInfo](index.md)

Timing information about a frame.

#### See also

| |
|---|
| [getFrameInfoHistory(FrameInfo[])](../get-frame-info-history.md) |

## Constructors

| | |
|---|---|
| [FrameInfo](-frame-info.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [backendBeginFrame](backend-begin-frame.md) | [main]<br>open var [backendBeginFrame](backend-begin-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Backend thread time of frame start since epoch [ns]. |
| [backendEndFrame](backend-end-frame.md) | [main]<br>open var [backendEndFrame](backend-end-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Backend thread time of frame end since epoch [ns]. |
| [beginFrame](begin-frame.md) | [main]<br>open var [beginFrame](begin-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Renderer.beginFrame() time since epoch [ns]. |
| [compositionToPresentLatency](composition-to-present-latency.md) | [main]<br>open var [compositionToPresentLatency](composition-to-present-latency.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Time between the start of composition and the expected present time [ns]. |
| [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md) | [main]<br>open var [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Denoised frame duration on the GPU in nanoseconds [ns]. |
| [displayPresent](display-present.md) | [main]<br>open var [displayPresent](display-present.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Actual presentation time of this frame since epoch [ns]. |
| [displayPresentInterval](display-present-interval.md) | [main]<br>open var [displayPresentInterval](display-present-interval.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Display refresh rate period [ns]. |
| [endFrame](end-frame.md) | [main]<br>open var [endFrame](end-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Renderer.endFrame() time since epoch [ns]. |
| [expectedPresentLatency](expected-present-latency.md) | [main]<br>open var [expectedPresentLatency](expected-present-latency.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Time between vsync and the system's expected presentation time [ns]. |
| [frameId](frame-id.md) | [main]<br>open var [frameId](frame-id.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Monotonically increasing frame identifier. |
| [frameScheduleTime](frame-schedule-time.md) | [main]<br>open var [frameScheduleTime](frame-schedule-time.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Frame scheduling callback entry time since epoch [ns]. |
| [gpuFrameComplete](gpu-frame-complete.md) | [main]<br>open var [gpuFrameComplete](gpu-frame-complete.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>GPU thread time of frame end since epoch [ns] or 0. |
| [gpuFrameDuration](gpu-frame-duration.md) | [main]<br>open var [gpuFrameDuration](gpu-frame-duration.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Frame duration on the GPU in nanoseconds [ns]. |
| [INVALID](-i-n-v-a-l-i-d.md) | [main]<br>val [INVALID](-i-n-v-a-l-i-d.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = -1<br>Value not supported or unavailable. |
| [PENDING](-p-e-n-d-i-n-g.md) | [main]<br>val [PENDING](-p-e-n-d-i-n-g.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = -2<br>Value not yet available (pending completion). |
| [presentDeadline](present-deadline.md) | [main]<br>open var [presentDeadline](present-deadline.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Deadline for queuing a frame [ns]. |
| [vsync](vsync.md) | [main]<br>open var [vsync](vsync.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>VSYNC hardware time of this frame since epoch [ns]. |
