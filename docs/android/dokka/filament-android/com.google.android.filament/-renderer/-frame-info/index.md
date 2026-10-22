//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[FrameInfo](index.md)

# FrameInfo

open class [FrameInfo](index.md)

Timing information about a frame

#### See also

| |
|---|
| [getFrameInfoHistory](../get-frame-info-history.md) |

## Constructors

| | |
|---|---|
| [FrameInfo](-frame-info.md) | [main]<br>constructor()constructor(frameId: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), gpuFrameDuration: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), denoisedGpuFrameDuration: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), beginFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), endFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), backendBeginFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), backendEndFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), gpuFrameComplete: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsync: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), displayPresent: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), presentDeadline: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), displayPresentInterval: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), compositionToPresentLatency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), expectedPresentLatency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), frameScheduleTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [backendBeginFrame](backend-begin-frame.md) | [main]<br>open var [backendBeginFrame](backend-begin-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [backendEndFrame](backend-end-frame.md) | [main]<br>open var [backendEndFrame](backend-end-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [beginFrame](begin-frame.md) | [main]<br>open var [beginFrame](begin-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [compositionToPresentLatency](composition-to-present-latency.md) | [main]<br>open var [compositionToPresentLatency](composition-to-present-latency.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md) | [main]<br>open var [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [displayPresent](display-present.md) | [main]<br>open var [displayPresent](display-present.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [displayPresentInterval](display-present-interval.md) | [main]<br>open var [displayPresentInterval](display-present-interval.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [endFrame](end-frame.md) | [main]<br>open var [endFrame](end-frame.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [expectedPresentLatency](expected-present-latency.md) | [main]<br>open var [expectedPresentLatency](expected-present-latency.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [frameId](frame-id.md) | [main]<br>open var [frameId](frame-id.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [frameScheduleTime](frame-schedule-time.md) | [main]<br>open var [frameScheduleTime](frame-schedule-time.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [gpuFrameComplete](gpu-frame-complete.md) | [main]<br>open var [gpuFrameComplete](gpu-frame-complete.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [gpuFrameDuration](gpu-frame-duration.md) | [main]<br>open var [gpuFrameDuration](gpu-frame-duration.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [INVALID](-i-n-v-a-l-i-d.md) | [main]<br>val [INVALID](-i-n-v-a-l-i-d.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = -1<br>value not supported |
| [PENDING](-p-e-n-d-i-n-g.md) | [main]<br>val [PENDING](-p-e-n-d-i-n-g.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) = -2<br>value not yet available |
| [presentDeadline](present-deadline.md) | [main]<br>open var [presentDeadline](present-deadline.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [vsync](vsync.md) | [main]<br>open var [vsync](vsync.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getBackendBeginFrame](get-backend-begin-frame.md) | [main]<br>open fun [getBackendBeginFrame](get-backend-begin-frame.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getBackendEndFrame](get-backend-end-frame.md) | [main]<br>open fun [getBackendEndFrame](get-backend-end-frame.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getBeginFrame](get-begin-frame.md) | [main]<br>open fun [getBeginFrame](get-begin-frame.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getCompositionToPresentLatency](get-composition-to-present-latency.md) | [main]<br>open fun [getCompositionToPresentLatency](get-composition-to-present-latency.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getDenoisedGpuFrameDuration](get-denoised-gpu-frame-duration.md) | [main]<br>open fun [getDenoisedGpuFrameDuration](get-denoised-gpu-frame-duration.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getDisplayPresent](get-display-present.md) | [main]<br>open fun [getDisplayPresent](get-display-present.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getDisplayPresentInterval](get-display-present-interval.md) | [main]<br>open fun [getDisplayPresentInterval](get-display-present-interval.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getEndFrame](get-end-frame.md) | [main]<br>open fun [getEndFrame](get-end-frame.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getExpectedPresentLatency](get-expected-present-latency.md) | [main]<br>open fun [getExpectedPresentLatency](get-expected-present-latency.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getFrameId](get-frame-id.md) | [main]<br>open fun [getFrameId](get-frame-id.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getFrameScheduleTime](get-frame-schedule-time.md) | [main]<br>open fun [getFrameScheduleTime](get-frame-schedule-time.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getGpuFrameComplete](get-gpu-frame-complete.md) | [main]<br>open fun [getGpuFrameComplete](get-gpu-frame-complete.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getGpuFrameDuration](get-gpu-frame-duration.md) | [main]<br>open fun [getGpuFrameDuration](get-gpu-frame-duration.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getPresentDeadline](get-present-deadline.md) | [main]<br>open fun [getPresentDeadline](get-present-deadline.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getVsync](get-vsync.md) | [main]<br>open fun [getVsync](get-vsync.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setBackendBeginFrame](set-backend-begin-frame.md) | [main]<br>open fun [setBackendBeginFrame](set-backend-begin-frame.md)(backendBeginFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setBackendEndFrame](set-backend-end-frame.md) | [main]<br>open fun [setBackendEndFrame](set-backend-end-frame.md)(backendEndFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setBeginFrame](set-begin-frame.md) | [main]<br>open fun [setBeginFrame](set-begin-frame.md)(beginFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setCompositionToPresentLatency](set-composition-to-present-latency.md) | [main]<br>open fun [setCompositionToPresentLatency](set-composition-to-present-latency.md)(compositionToPresentLatency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setDenoisedGpuFrameDuration](set-denoised-gpu-frame-duration.md) | [main]<br>open fun [setDenoisedGpuFrameDuration](set-denoised-gpu-frame-duration.md)(denoisedGpuFrameDuration: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setDisplayPresent](set-display-present.md) | [main]<br>open fun [setDisplayPresent](set-display-present.md)(displayPresent: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setDisplayPresentInterval](set-display-present-interval.md) | [main]<br>open fun [setDisplayPresentInterval](set-display-present-interval.md)(displayPresentInterval: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setEndFrame](set-end-frame.md) | [main]<br>open fun [setEndFrame](set-end-frame.md)(endFrame: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setExpectedPresentLatency](set-expected-present-latency.md) | [main]<br>open fun [setExpectedPresentLatency](set-expected-present-latency.md)(expectedPresentLatency: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setFrameId](set-frame-id.md) | [main]<br>open fun [setFrameId](set-frame-id.md)(frameId: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [setFrameScheduleTime](set-frame-schedule-time.md) | [main]<br>open fun [setFrameScheduleTime](set-frame-schedule-time.md)(frameScheduleTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setGpuFrameComplete](set-gpu-frame-complete.md) | [main]<br>open fun [setGpuFrameComplete](set-gpu-frame-complete.md)(gpuFrameComplete: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setGpuFrameDuration](set-gpu-frame-duration.md) | [main]<br>open fun [setGpuFrameDuration](set-gpu-frame-duration.md)(gpuFrameDuration: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setPresentDeadline](set-present-deadline.md) | [main]<br>open fun [setPresentDeadline](set-present-deadline.md)(presentDeadline: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setVsync](set-vsync.md) | [main]<br>open fun [setVsync](set-vsync.md)(vsync: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
