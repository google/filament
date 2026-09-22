//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[beginFrame](begin-frame.md)

# beginFrame

[main]\
open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Set up a frame for this Renderer. 

beginFrame() manages frame-pacing, and returns whether a frame should be drawn. The goal of this is to skip frames when the GPU falls behind in order to keep the frame latency low.

If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The display will draw the same frame twice producing a stutter. At this point, the CPU is ahead of the GPU and depending on how many frames are buffered, latency increases.

beginFrame() attempts to detect this situation and returns false in that case, indicating to the caller to skip the current frame.

When beginFrame() returns true, it is mandatory to render the frame and call endFrame(). However, when beginFrame() returns false, the caller has the choice to either skip the frame and not call endFrame(), or proceed as though true was returned.

All calls to render() must happen *after* beginFrame(). It is recommended to use the same swapChain for every call to beginFrame, failing to do so can result is losing all or part of the FrameInfo history.

This method will return false if called again after a backend exception was already thrown and delivered to the main thread.

#### Return

*false* the current frame should be skipped, *true* the current frame must be drawn and endFrame() must be called.

@remark When skipping a frame, the whole frame is canceled, and endFrame() must not be called.

#### Parameters

main

| | |
|---|---|
| swapChain | A pointer to the SwapChain instance to use. |

#### See also

| |
|---|
| [endFrame](end-frame.md) |

[main]\
open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md), vsyncSteadyClockTimeNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Set up a frame for this Renderer. 

beginFrame() manages frame-pacing, and returns whether a frame should be drawn. The goal of this is to skip frames when the GPU falls behind in order to keep the frame latency low.

If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The display will draw the same frame twice producing a stutter. At this point, the CPU is ahead of the GPU and depending on how many frames are buffered, latency increases.

beginFrame() attempts to detect this situation and returns false in that case, indicating to the caller to skip the current frame.

When beginFrame() returns true, it is mandatory to render the frame and call endFrame(). However, when beginFrame() returns false, the caller has the choice to either skip the frame and not call endFrame(), or proceed as though true was returned.

All calls to render() must happen *after* beginFrame(). It is recommended to use the same swapChain for every call to beginFrame, failing to do so can result is losing all or part of the FrameInfo history.

This method will return false if called again after a backend exception was already thrown and delivered to the main thread.

#### Return

*false* the current frame should be skipped, *true* the current frame must be drawn and endFrame() must be called.

@remark When skipping a frame, the whole frame is canceled, and endFrame() must not be called.

#### Parameters

main

| | |
|---|---|
| swapChain | A pointer to the SwapChain instance to use. |
| vsyncSteadyClockTimeNano | The time in nanosecond of when the current frame started, or 0 if unknown. This value should be the timestamp of the last h/w vsync. It is expressed in the std::chrono::steady_clock time base. On Android this should be the frame time received from a Choreographer. |

#### See also

| |
|---|
| [endFrame](end-frame.md) |
