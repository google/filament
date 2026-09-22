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
| [FrameInfo](-frame-info.md) | [main]<br>constructor()constructor(frameId: Int, gpuFrameDuration: Long, denoisedGpuFrameDuration: Long, beginFrame: Long, endFrame: Long, backendBeginFrame: Long, backendEndFrame: Long, gpuFrameComplete: Long, vsync: Long, displayPresent: Long, presentDeadline: Long, displayPresentInterval: Long, compositionToPresentLatency: Long, expectedPresentLatency: Long, frameScheduleTime: Long) |

## Properties

| Name | Summary |
|---|---|
| [backendBeginFrame](backend-begin-frame.md) | [main]<br>open var [backendBeginFrame](backend-begin-frame.md): Long |
| [backendEndFrame](backend-end-frame.md) | [main]<br>open var [backendEndFrame](backend-end-frame.md): Long |
| [beginFrame](begin-frame.md) | [main]<br>open var [beginFrame](begin-frame.md): Long |
| [compositionToPresentLatency](composition-to-present-latency.md) | [main]<br>open var [compositionToPresentLatency](composition-to-present-latency.md): Long |
| [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md) | [main]<br>open var [denoisedGpuFrameDuration](denoised-gpu-frame-duration.md): Long |
| [displayPresent](display-present.md) | [main]<br>open var [displayPresent](display-present.md): Long |
| [displayPresentInterval](display-present-interval.md) | [main]<br>open var [displayPresentInterval](display-present-interval.md): Long |
| [endFrame](end-frame.md) | [main]<br>open var [endFrame](end-frame.md): Long |
| [expectedPresentLatency](expected-present-latency.md) | [main]<br>open var [expectedPresentLatency](expected-present-latency.md): Long |
| [frameId](frame-id.md) | [main]<br>open var [frameId](frame-id.md): Int |
| [frameScheduleTime](frame-schedule-time.md) | [main]<br>open var [frameScheduleTime](frame-schedule-time.md): Long |
| [gpuFrameComplete](gpu-frame-complete.md) | [main]<br>open var [gpuFrameComplete](gpu-frame-complete.md): Long |
| [gpuFrameDuration](gpu-frame-duration.md) | [main]<br>open var [gpuFrameDuration](gpu-frame-duration.md): Long |
| [INVALID](-i-n-v-a-l-i-d.md) | [main]<br>val [INVALID](-i-n-v-a-l-i-d.md): Long = -1<br>value not supported |
| [PENDING](-p-e-n-d-i-n-g.md) | [main]<br>val [PENDING](-p-e-n-d-i-n-g.md): Long = -2<br>value not yet available |
| [presentDeadline](present-deadline.md) | [main]<br>open var [presentDeadline](present-deadline.md): Long |
| [vsync](vsync.md) | [main]<br>open var [vsync](vsync.md): Long |

## Functions

| Name | Summary |
|---|---|
| [getBackendBeginFrame](get-backend-begin-frame.md) | [main]<br>open fun [getBackendBeginFrame](get-backend-begin-frame.md)(): Long |
| [getBackendEndFrame](get-backend-end-frame.md) | [main]<br>open fun [getBackendEndFrame](get-backend-end-frame.md)(): Long |
| [getBeginFrame](get-begin-frame.md) | [main]<br>open fun [getBeginFrame](get-begin-frame.md)(): Long |
| [getCompositionToPresentLatency](get-composition-to-present-latency.md) | [main]<br>open fun [getCompositionToPresentLatency](get-composition-to-present-latency.md)(): Long |
| [getDenoisedGpuFrameDuration](get-denoised-gpu-frame-duration.md) | [main]<br>open fun [getDenoisedGpuFrameDuration](get-denoised-gpu-frame-duration.md)(): Long |
| [getDisplayPresent](get-display-present.md) | [main]<br>open fun [getDisplayPresent](get-display-present.md)(): Long |
| [getDisplayPresentInterval](get-display-present-interval.md) | [main]<br>open fun [getDisplayPresentInterval](get-display-present-interval.md)(): Long |
| [getEndFrame](get-end-frame.md) | [main]<br>open fun [getEndFrame](get-end-frame.md)(): Long |
| [getExpectedPresentLatency](get-expected-present-latency.md) | [main]<br>open fun [getExpectedPresentLatency](get-expected-present-latency.md)(): Long |
| [getFrameId](get-frame-id.md) | [main]<br>open fun [getFrameId](get-frame-id.md)(): Int |
| [getFrameScheduleTime](get-frame-schedule-time.md) | [main]<br>open fun [getFrameScheduleTime](get-frame-schedule-time.md)(): Long |
| [getGpuFrameComplete](get-gpu-frame-complete.md) | [main]<br>open fun [getGpuFrameComplete](get-gpu-frame-complete.md)(): Long |
| [getGpuFrameDuration](get-gpu-frame-duration.md) | [main]<br>open fun [getGpuFrameDuration](get-gpu-frame-duration.md)(): Long |
| [getPresentDeadline](get-present-deadline.md) | [main]<br>open fun [getPresentDeadline](get-present-deadline.md)(): Long |
| [getVsync](get-vsync.md) | [main]<br>open fun [getVsync](get-vsync.md)(): Long |
| [setBackendBeginFrame](set-backend-begin-frame.md) | [main]<br>open fun [setBackendBeginFrame](set-backend-begin-frame.md)(backendBeginFrame: Long) |
| [setBackendEndFrame](set-backend-end-frame.md) | [main]<br>open fun [setBackendEndFrame](set-backend-end-frame.md)(backendEndFrame: Long) |
| [setBeginFrame](set-begin-frame.md) | [main]<br>open fun [setBeginFrame](set-begin-frame.md)(beginFrame: Long) |
| [setCompositionToPresentLatency](set-composition-to-present-latency.md) | [main]<br>open fun [setCompositionToPresentLatency](set-composition-to-present-latency.md)(compositionToPresentLatency: Long) |
| [setDenoisedGpuFrameDuration](set-denoised-gpu-frame-duration.md) | [main]<br>open fun [setDenoisedGpuFrameDuration](set-denoised-gpu-frame-duration.md)(denoisedGpuFrameDuration: Long) |
| [setDisplayPresent](set-display-present.md) | [main]<br>open fun [setDisplayPresent](set-display-present.md)(displayPresent: Long) |
| [setDisplayPresentInterval](set-display-present-interval.md) | [main]<br>open fun [setDisplayPresentInterval](set-display-present-interval.md)(displayPresentInterval: Long) |
| [setEndFrame](set-end-frame.md) | [main]<br>open fun [setEndFrame](set-end-frame.md)(endFrame: Long) |
| [setExpectedPresentLatency](set-expected-present-latency.md) | [main]<br>open fun [setExpectedPresentLatency](set-expected-present-latency.md)(expectedPresentLatency: Long) |
| [setFrameId](set-frame-id.md) | [main]<br>open fun [setFrameId](set-frame-id.md)(frameId: Int) |
| [setFrameScheduleTime](set-frame-schedule-time.md) | [main]<br>open fun [setFrameScheduleTime](set-frame-schedule-time.md)(frameScheduleTime: Long) |
| [setGpuFrameComplete](set-gpu-frame-complete.md) | [main]<br>open fun [setGpuFrameComplete](set-gpu-frame-complete.md)(gpuFrameComplete: Long) |
| [setGpuFrameDuration](set-gpu-frame-duration.md) | [main]<br>open fun [setGpuFrameDuration](set-gpu-frame-duration.md)(gpuFrameDuration: Long) |
| [setPresentDeadline](set-present-deadline.md) | [main]<br>open fun [setPresentDeadline](set-present-deadline.md)(presentDeadline: Long) |
| [setVsync](set-vsync.md) | [main]<br>open fun [setVsync](set-vsync.md)(vsync: Long) |
