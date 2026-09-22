//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[copyFrame](copy-frame.md)

# copyFrame

[main]\
open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md))

Copy the currently rendered view to the indicated swap chain, using the indicated source and destination rectangle.

#### Parameters

main

| | |
|---|---|
| dstSwapChain | The swap chain into which the frame should be copied. |
| dstViewport | The destination rectangle in which to draw the view. |
| srcViewport | The source rectangle to be copied. |

[main]\
open fun [copyFrame](copy-frame.md)(dstSwapChain: [SwapChain](../-swap-chain/index.md), dstViewport: [Viewport](../-viewport/index.md), srcViewport: [Viewport](../-viewport/index.md), flags: Int)

Copy the currently rendered view to the indicated swap chain, using the indicated source and destination rectangle.

#### Parameters

main

| | |
|---|---|
| dstSwapChain | The swap chain into which the frame should be copied. |
| dstViewport | The destination rectangle in which to draw the view. |
| srcViewport | The source rectangle to be copied. |
| flags | One or more CopyFrameFlag behavior configuration flags.<br>@remark copyFrame() should be called after a frame is rendered using render() but before endFrame() is called. |
