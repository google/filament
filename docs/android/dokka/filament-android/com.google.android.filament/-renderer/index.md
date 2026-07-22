//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)

# Renderer

open class [Renderer](index.md)

A `Renderer` instance represents an operating system's window. 

 Typically, applications create a `Renderer` per window. The `Renderer` generates drawing commands for the render thread and manages frame latency.  A Renderer generates drawing commands from a View, itself containing a Scene description. 

# Creation and Destruction

A `Renderer` is created using [createRenderer](../-engine/create-renderer.md) and destroyed using [destroyRenderer](../-engine/destroy-renderer.md).

#### See also

| |
|---|
| [Engine](../-engine/index.md) |
| [View](../-view/index.md) |

## Types

| Name | Summary |
|---|---|
| [ClearOptions](-clear-options/index.md) | [main]<br>open class [ClearOptions](-clear-options/index.md)<br>ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content. |
| [DisplayInfo](-display-info/index.md) | [main]<br>open class [DisplayInfo](-display-info/index.md)<br>Information about the display this renderer is associated to |
| [FrameInfo](-frame-info/index.md) | [main]<br>open class [FrameInfo](-frame-info/index.md)<br>Timing information about a frame. |
| [FrameRateOptions](-frame-rate-options/index.md) | [main]<br>open class [FrameRateOptions](-frame-rate-options/index.md)<br>Use FrameRateOptions to set the desired frame rate and control how quickly the system reacts to GPU load changes. |

## Properties

| Name | Summary |
|---|---|
| [MIRROR_FRAME_FLAG_CLEAR](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-c-l-e-a-r.md) | [main]<br>val [MIRROR_FRAME_FLAG_CLEAR](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-c-l-e-a-r.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 4<br>Indicates that the `dstSwapChain` passed into [copyFrame](copy-frame.md) should be cleared to black before the frame is copied into the specified viewport. |
| [MIRROR_FRAME_FLAG_COMMIT](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-c-o-m-m-i-t.md) | [main]<br>val [MIRROR_FRAME_FLAG_COMMIT](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-c-o-m-m-i-t.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 1<br>Indicates that the `dstSwapChain` passed into [copyFrame](copy-frame.md) should be committed after the frame has been copied. |
| [MIRROR_FRAME_FLAG_SET_PRESENTATION_TIME](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-s-e-t_-p-r-e-s-e-n-t-a-t-i-o-n_-t-i-m-e.md) | [main]<br>val [MIRROR_FRAME_FLAG_SET_PRESENTATION_TIME](-m-i-r-r-o-r_-f-r-a-m-e_-f-l-a-g_-s-e-t_-p-r-e-s-e-n-t-a-t-i-o-n_-t-i-m-e.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 2<br>Indicates that the presentation time should be set on the `dstSwapChain` passed into [copyFrame](copy-frame.md) to the monotonic clock time when the frame is copied. |

## Functions

| Name | Summary |
|---|---|
| [beginFrame](begin-frame.md) | [main]<br>open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md), frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Sets up a frame for this `Renderer`. |
| [copyFrame](copy-frame.md) | [main]<br>open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md), flags: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Copies the currently rendered [View](../-view/index.md) to the indicated [SwapChain](../-swap-chain/index.md), using the indicated source and destination rectangle. |
| [endFrame](end-frame.md) | [main]<br>open fun [endFrame](end-frame.md)()<br>Finishes the current frame and schedules it for display. |
| [getClearOptions](get-clear-options.md) | [main]<br>open fun [getClearOptions](get-clear-options.md)(): [Renderer.ClearOptions](-clear-options/index.md)<br>Returns the ClearOptions object set in [setClearOptions](set-clear-options.md) or a new instance otherwise. |
| [getDisplayInfo](get-display-info.md) | [main]<br>open fun [getDisplayInfo](get-display-info.md)(): [Renderer.DisplayInfo](-display-info/index.md)<br>Returns the DisplayInfo object set in [setDisplayInfo](set-display-info.md) or a new instance otherwise. |
| [getEngine](get-engine.md) | [main]<br>open fun [getEngine](get-engine.md)(): [Engine](../-engine/index.md)<br>Gets the [Engine](../-engine/index.md) that created this `Renderer`. |
| [getFrameInfoHistory](get-frame-info-history.md) | [main]<br>open fun [getFrameInfoHistory](get-frame-info-history.md)(outHistory: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Renderer.FrameInfo](-frame-info/index.md)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Retrieve a history of frame timing information. |
| [getFrameRateOptions](get-frame-rate-options.md) | [main]<br>open fun [getFrameRateOptions](get-frame-rate-options.md)(): [Renderer.FrameRateOptions](-frame-rate-options/index.md)<br>Returns the FrameRateOptions object set in [setFrameRateOptions](set-frame-rate-options.md) or a new instance otherwise. |
| [getFrameToSkipCount](get-frame-to-skip-count.md) | [main]<br>open fun [getFrameToSkipCount](get-frame-to-skip-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Remainder count of frame to be skipped |
| [getMaterialTime](get-material-time.md) | [main]<br>open fun [getMaterialTime](get-material-time.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)<br>Returns a timestamp (in seconds) for the last call to [beginFrame](begin-frame.md). |
| [getMaxFrameHistorySize](get-max-frame-history-size.md) | [main]<br>open fun [getMaxFrameHistorySize](get-max-frame-history-size.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getUserTime](get-user-time.md) | [main]<br>open fun [~~getUserTime~~](get-user-time.md)(): [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)<br>Backward compatibility helper for getUserTime(). |
| [hasGpuFallenBehind](has-gpu-fallen-behind.md) | [main]<br>open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Queries whether the GPU execution has fallen behind the CPU rendering execution. |
| [mirrorFrame](mirror-frame.md) | [main]<br>open fun [~~mirrorFrame~~](mirror-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md), flags: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |
| [pauseRenderThread](pause-render-thread.md) | [main]<br>open fun [pauseRenderThread](pause-render-thread.md)(durationNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Stalls the render thread (GPU submission pipeline) for the given duration in nanoseconds. |
| [readPixels](read-pixels.md) | [main]<br>open fun [readPixels](read-pixels.md)(xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](../-texture/-pixel-buffer-descriptor/index.md))<br>Reads back the content of the [SwapChain](../-swap-chain/index.md) associated with this `Renderer`.<br>[main]<br>open fun [readPixels](read-pixels.md)(renderTarget: [RenderTarget](../-render-target/index.md), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](../-texture/-pixel-buffer-descriptor/index.md))<br>Reads back the content of a specified [RenderTarget](../-render-target/index.md). |
| [render](render.md) | [main]<br>open fun [render](render.md)(view: [View](../-view/index.md))<br>Renders a [View](../-view/index.md) into this `Renderer`'s window. |
| [renderStandaloneView](render-standalone-view.md) | [main]<br>open fun [renderStandaloneView](render-standalone-view.md)(view: [View](../-view/index.md))<br>Renders a standalone [View](../-view/index.md) into its associated `RenderTarget`. |
| [resetUserTime](reset-user-time.md) | [main]<br>open fun [~~resetUserTime~~](reset-user-time.md)()<br>Backward compatibility helper for resetUserTime(). |
| [setClearOptions](set-clear-options.md) | [main]<br>open fun [setClearOptions](set-clear-options.md)(options: [Renderer.ClearOptions](-clear-options/index.md))<br>Set ClearOptions which are used at the beginning of a frame to clear or retain the SwapChain content. |
| [setDesiredPresentationTime](set-desired-presentation-time.md) | [main]<br>open fun [setDesiredPresentationTime](set-desired-presentation-time.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Set the real desired presentation time targeted for this frame. |
| [setDisplayInfo](set-display-info.md) | [main]<br>open fun [setDisplayInfo](set-display-info.md)(info: [Renderer.DisplayInfo](-display-info/index.md))<br>Information about the display this Renderer is associated to. |
| [setFrameRateOptions](set-frame-rate-options.md) | [main]<br>open fun [setFrameRateOptions](set-frame-rate-options.md)(options: [Renderer.FrameRateOptions](-frame-rate-options/index.md))<br>Set options controlling the desired frame-rate. |
| [setFrameScheduleTime](set-frame-schedule-time.md) | [main]<br>open fun [setFrameScheduleTime](set-frame-schedule-time.md)(timeSteadyClockNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Sets the physical clock time when the frame scheduling callback was entered. |
| [setMaterialTimeEpoch](set-material-time-epoch.md) | [main]<br>open fun [setMaterialTimeEpoch](set-material-time-epoch.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Sets the material time epoch to the specified steady clock timestamp in nanoseconds, i.e. |
| [setPresentationTime](set-presentation-time.md) | [main]<br>open fun [setPresentationTime](set-presentation-time.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Set the time at which the frame must be presented to the display hardware. |
| [setRenderingDeadline](set-rendering-deadline.md) | [main]<br>open fun [setRenderingDeadline](set-rendering-deadline.md)(monotonicClockNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Set the deadline timestamp on the steady clock by which CPU and GPU rendering must complete for the buffer to meet its target display latching window. |
| [setVsyncTime](set-vsync-time.md) | [main]<br>open fun [setVsyncTime](set-vsync-time.md)(steadyClockTimeNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>The use of this method is optional. |
| [shouldRenderFrame](should-render-frame.md) | [main]<br>open fun [shouldRenderFrame](should-render-frame.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns true if the current frame should be rendered. |
| [skipFrame](skip-frame.md) | [main]<br>open fun [skipFrame](skip-frame.md)(vsyncSteadyClockTimeNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Call skipFrame when momentarily skipping frames, for instance if the content of the scene doesn't change. |
| [skipNextFrames](skip-next-frames.md) | [main]<br>open fun [skipNextFrames](skip-next-frames.md)(frameCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Requests the next frameCount frames to be skipped. |
