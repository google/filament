//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[beginFrame](begin-frame.md)

# beginFrame

[main]\
open fun [beginFrame](begin-frame.md)(swapChain: [SwapChain](../-swap-chain/index.md), frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Sets up a frame for this `Renderer`. 

`beginFrame` manages frame pacing, and returns whether or not a frame should be drawn. The goal of this is to skip frames when the GPU falls behind in order to keep the frame latency low.

If a given frame takes too much time in the GPU, the CPU will get ahead of the GPU. The display will draw the same frame twice producing a stutter. At this point, the CPU is ahead of the GPU and depending on how many frames are buffered, latency increases. beginFrame() attempts to detect this situation and returns `false` in that case, indicating to the caller to skip the current frame.

All calls to render() must happen **after** beginFrame().

#### Return

`true`: the current frame must be drawn, and [endFrame](end-frame.md) must be called`false`: the current frame should be skipped, when skipping a frame, the whole frame is canceled, and [endFrame](end-frame.md) must not be called. However, the user can choose to proceed as though `true` was returned and produce a frame anyways, by making calls to [render](render.md), in which case [endFrame](end-frame.md) must be called.

#### Parameters

main

| | |
|---|---|
| swapChain | the [SwapChain](../-swap-chain/index.md) instance to use |
| frameTimeNanos | The time in nanoseconds when the frame started being rendered, in the [nanoTime](https://developer.android.com/reference/kotlin/java/lang/System.html#nanotime) timebase. Divide this value by 1000000 to convert it to the uptimeMillis time base. This typically comes from android.view.Choreographer.FrameCallback. |

#### See also

| |
|---|
| [endFrame](end-frame.md) |
| [render](render.md) |

#### Throws

| | |
|---|---|
| [Error](https://developer.android.com/reference/kotlin/java/lang/Error.html) | if the backend thread encountered an unrecoverable error. |
