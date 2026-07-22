//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[copyFrame](copy-frame.md)

# copyFrame

[main]\
open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md), flags: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Copies the currently rendered [View](../-view/index.md) to the indicated [SwapChain](../-swap-chain/index.md), using the indicated source and destination rectangle. 

`copyFrame()` should be called after a frame is rendered using [render](render.md) but before [endFrame](end-frame.md) is called.

#### Parameters

main

| | |
|---|---|
| dstSwapChain | the [SwapChain](../-swap-chain/index.md) into which the frame should be copied |
| dstViewport | the destination rectangle in which to draw the view |
| srcViewport | the source rectangle to be copied |
| flags | one or more `CopyFrameFlag` behavior configuration flags |
