//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)

# Renderer

open class [Renderer](index.md)

A Renderer instance represents an operating system's window. 

Typically, applications create a Renderer per window. The Renderer generates drawing commands for the render thread and manages frame latency.

A Renderer generates drawing commands from a View, itself containing a Scene description.

# Creation and Destruction

A Renderer is created using Engine.createRenderer() and destroyed using Engine.destroy(const Renderer*).

```kotlin

#include <filament/Renderer.h>
#include <filament/Engine.h>
using namespace filament;

Engine* engine = Engine::create();

Renderer* renderer = engine->createRenderer();
engine->destroy(&renderer);

```

#### See also

| |
|---|
| [Engine](../-engine/index.md) |
| [View](../-view/index.md) |

## Types

| Name | Summary |
|---|---|
| [ClearOptions](-clear-options/index.md) | [main]<br>open class [ClearOptions](-clear-options/index.md)<br>ClearOptions are used at the beginning of a frame to clear or retain the SwapChain content. |
| [DisplayInfo](-display-info/index.md) | [main]<br>open class [DisplayInfo](-display-info/index.md)<br>Use DisplayInfo to set important Display properties. |
| [FrameInfo](-frame-info/index.md) | [main]<br>open class [FrameInfo](-frame-info/index.md)<br>Timing information about a frame |
| [FrameRateOptions](-frame-rate-options/index.md) | [main]<br>open class [FrameRateOptions](-frame-rate-options/index.md)<br>Use FrameRateOptions to set the desired frame rate and control how quickly the system reacts to GPU load changes. |

## Properties

| Name | Summary |
|---|---|
| [CLEAR](-c-l-e-a-r.md) | [main]<br>val [CLEAR](-c-l-e-a-r.md): Int = 4<br>Indicates that the dstSwapChain passed into copyFrame() should be cleared to black before the frame is copied into the specified viewport. |
| [COMMIT](-c-o-m-m-i-t.md) | [main]<br>val [COMMIT](-c-o-m-m-i-t.md): Int = 1<br>Indicates that the dstSwapChain passed into copyFrame() should be committed after the frame has been copied. |
| [SET_PRESENTATION_TIME](-s-e-t_-p-r-e-s-e-n-t-a-t-i-o-n_-t-i-m-e.md) | [main]<br>val [SET_PRESENTATION_TIME](-s-e-t_-p-r-e-s-e-n-t-a-t-i-o-n_-t-i-m-e.md): Int = 2<br>Indicates that the presentation time should be set on the dstSwapChain passed into copyFrame to the monotonic clock time when the frame is copied. |

## Functions

| Name | Summary |
|---|---|
| [beginFrame](begin-frame.md) | [main]<br>open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md)): Boolean<br>open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md), vsyncSteadyClockTimeNano: Long): Boolean<br>Set up a frame for this Renderer. |
| [copyFrame](copy-frame.md) | [main]<br>open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md))<br>open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md), flags: Int)<br>Copy the currently rendered view to the indicated swap chain, using the indicated source and destination rectangle. |
| [endFrame](end-frame.md) | [main]<br>open fun [endFrame](end-frame.md)()<br>Finishes the current frame and schedules it for display. |
| [getClearOptions](get-clear-options.md) | [main]<br>open fun [getClearOptions](get-clear-options.md)(): [Renderer.ClearOptions](-clear-options/index.md)<br>Returns the ClearOptions currently set. |
| [getDisplayInfo](get-display-info.md) | [main]<br>open fun [getDisplayInfo](get-display-info.md)(): [Renderer.DisplayInfo](-display-info/index.md) |
| [getEngine](get-engine.md) | [main]<br>open fun [getEngine](get-engine.md)(): [Engine](../-engine/index.md)<br>Get the Engine that created this Renderer. |
| [getFrameInfoHistory](get-frame-info-history.md) | [main]<br>open fun [getFrameInfoHistory](get-frame-info-history.md)(outHistory: Array&lt;[Renderer.FrameInfo](-frame-info/index.md)&gt;): Int<br>Retrieve a history of frame timing information. |
| [getFrameRateOptions](get-frame-rate-options.md) | [main]<br>open fun [getFrameRateOptions](get-frame-rate-options.md)(): [Renderer.FrameRateOptions](-frame-rate-options/index.md) |
| [getFrameToSkipCount](get-frame-to-skip-count.md) | [main]<br>open fun [getFrameToSkipCount](get-frame-to-skip-count.md)(): Int<br>Remainder count of frame to be skipped |
| [getMaterialTime](get-material-time.md) | [main]<br>open fun [getMaterialTime](get-material-time.md)(): Double<br>Returns the material time in seconds evaluated for the current frame. |
| [getMaxFrameHistorySize](get-max-frame-history-size.md) | [main]<br>open fun [getMaxFrameHistorySize](get-max-frame-history-size.md)(): Int |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getUserTime](get-user-time.md) | [main]<br>open fun [getUserTime](get-user-time.md)(): Double<br>Backward compatibility helper for getUserTime(). |
| [hasGpuFallenBehind](has-gpu-fallen-behind.md) | [main]<br>open fun [hasGpuFallenBehind](has-gpu-fallen-behind.md)(): Boolean<br>Queries whether the GPU execution has fallen behind the CPU rendering execution. |
| [pauseRenderThread](pause-render-thread.md) | [main]<br>open fun [pauseRenderThread](pause-render-thread.md)(duration_ns: Long)<br>Stalls the render thread (GPU submission pipeline) for the given duration in nanoseconds. |
| [readPixels](read-pixels.md) | [main]<br>open fun [readPixels](read-pixels.md)(xoffset: Int, yoffset: Int, width: Int, height: Int, buffer: [PixelBufferDescriptor](../-pixel-buffer-descriptor/index.md))<br>Reads back the content of the SwapChain associated with this Renderer.<br>[main]<br>open fun [readPixels](read-pixels.md)(renderTarget: [RenderTarget](../-render-target/index.md), xoffset: Int, yoffset: Int, width: Int, height: Int, buffer: [PixelBufferDescriptor](../-pixel-buffer-descriptor/index.md))<br>Reads back the content of the provided RenderTarget. |
| [render](render.md) | [main]<br>open fun [render](render.md)(view: [View](../-view/index.md))<br>Render a View into this renderer's window. |
| [renderStandaloneView](render-standalone-view.md) | [main]<br>open fun [renderStandaloneView](render-standalone-view.md)(view: [View](../-view/index.md))<br>Render a standalone View into its associated RenderTarget This call is mostly equivalent to calling render(View*) inside a beginFrame / endFrame block, but incurs less overhead. |
| [resetUserTime](reset-user-time.md) | [main]<br>open fun [resetUserTime](reset-user-time.md)()<br>Backward compatibility helper for resetUserTime(). |
| [setClearOptions](set-clear-options.md) | [main]<br>open fun [setClearOptions](set-clear-options.md)(options: [Renderer.ClearOptions](-clear-options/index.md))<br>Set ClearOptions which are used at the beginning of a frame to clear or retain the SwapChain content. |
| [setDesiredPresentationTime](set-desired-presentation-time.md) | [main]<br>open fun [setDesiredPresentationTime](set-desired-presentation-time.md)(monotonic_clock_ns: Long)<br>Set the real desired presentation time targeted for this frame. |
| [setDisplayInfo](set-display-info.md) | [main]<br>open fun [setDisplayInfo](set-display-info.md)(info: [Renderer.DisplayInfo](-display-info/index.md))<br>Information about the display this Renderer is associated to. |
| [setFrameRateOptions](set-frame-rate-options.md) | [main]<br>open fun [setFrameRateOptions](set-frame-rate-options.md)(options: [Renderer.FrameRateOptions](-frame-rate-options/index.md))<br>Set options controlling the desired frame-rate. |
| [setFrameScheduleTime](set-frame-schedule-time.md) | [main]<br>open fun [setFrameScheduleTime](set-frame-schedule-time.md)(time: Long)<br>Sets the physical clock time when the frame scheduling callback was entered. |
| [setMaterialTimeEpoch](set-material-time-epoch.md) | [main]<br>open fun [setMaterialTimeEpoch](set-material-time-epoch.md)(monotonic_clock_ns: Long)<br>Sets the material time epoch to the specified steady clock timestamp in nanoseconds, i.e. |
| [setPresentationTime](set-presentation-time.md) | [main]<br>open fun [setPresentationTime](set-presentation-time.md)(monotonic_clock_ns: Long)<br>Set the time at which the frame must be presented to the display hardware. |
| [setRenderingDeadline](set-rendering-deadline.md) | [main]<br>open fun [setRenderingDeadline](set-rendering-deadline.md)(monotonic_clock_ns: Long)<br>Set the deadline time point on the steady clock by which CPU and GPU rendering must complete for the buffer to meet its target display latching window. |
| [setVsyncTime](set-vsync-time.md) | [main]<br>open fun [setVsyncTime](set-vsync-time.md)(steadyClockTimeNano: Long)<br>The use of this method is optional. |
| [shouldRenderFrame](should-render-frame.md) | [main]<br>open fun [shouldRenderFrame](should-render-frame.md)(): Boolean<br>Returns true if the current frame should be rendered. |
| [skipFrame](skip-frame.md) | [main]<br>open fun [skipFrame](skip-frame.md)()<br>open fun [skipFrame](skip-frame.md)(vsyncSteadyClockTimeNano: Long)<br>Call skipFrame when momentarily skipping frames, for instance if the content of the scene doesn't change. |
| [skipNextFrames](skip-next-frames.md) | [main]<br>open fun [skipNextFrames](skip-next-frames.md)(frameCount: Int)<br>Requests the next frameCount frames to be skipped. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long, engine: [Engine](../-engine/index.md)): [Renderer](index.md) |
