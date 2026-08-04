//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[readPixels](read-pixels.md)

# readPixels

[main]\
open fun [readPixels](read-pixels.md)(xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](../-texture/-pixel-buffer-descriptor/index.md))

Reads back the content of the [SwapChain](../-swap-chain/index.md) associated with this `Renderer`. 

```kotlin

 Framebuffer as seen on         User buffer (PixelBufferDescriptor)
 screen
 +--------------------+
 |                    |                .stride         .alignment
 |                    |         ----------------------->-->
 |                    |         O----------------------+--+   low addresses
 |                    |         |          |           |  |
 |             w      |         |          | .top      |  |
 |       <--------->  |         |          V           |  |
 |       +---------+  |         |     +---------+      |  |
 |       |     ^   |  | ======> |     |         |      |  |
 |   x   |    h|   |  |         |.left|         |      |  |
 +------>|     v   |  |         +---->|         |      |  |
 |       +.........+  |         |     +.........+      |  |
 |            ^       |         |                      |  |
 |          y |       |         +----------------------+--+  high addresses
 O------------+-------+

```

`readPixels` must be called within a frame, meaning after [beginFrame](begin-frame.md) and before [endFrame](end-frame.md). Typically, `readPixels` will be called after [render](render.md).

After issuing this method, the callback associated with `buffer` will be invoked on the main thread, indicating that the read-back has completed. Typically, this will happen after multiple calls to [beginFrame](begin-frame.md), [render](render.md), [endFrame](end-frame.md).

It is also possible to use a [Fence](../-fence/index.md) to wait for the read-back.

`readPixels` is intended for debugging and testing. It will impact performance significantly.

#### Parameters

main

| | |
|---|---|
| xoffset | left offset of the sub-region to read back |
| yoffset | bottom offset of the sub-region to read back |
| width | width of the sub-region to read back |
| height | height of the sub-region to read back |
| buffer | client-side buffer where the read-back will be written <br>The following format are always supported: <br>[RGBA](../-texture/-format/-r-g-b-a/index.md)[RGBA_INTEGER](../-texture/-format/-r-g-b-a_-i-n-t-e-g-e-r/index.md)<br>The following types are always supported: <br>[UBYTE](../-texture/-type/-u-b-y-t-e/index.md)[UINT](../-texture/-type/-u-i-n-t/index.md)[INT](../-texture/-type/-i-n-t/index.md)[FLOAT](../-texture/-type/-f-l-o-a-t/index.md)<br>Other combination of format/type may be supported. If a combination is not supported, this operation may fail silently. Use a DEBUG build to get some logs about the failure. |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |

[main]\
open fun [readPixels](read-pixels.md)(renderTarget: [RenderTarget](../-render-target/index.md), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](../-texture/-pixel-buffer-descriptor/index.md))

Reads back the content of a specified [RenderTarget](../-render-target/index.md). 

```kotlin

 Framebuffer as seen on         User buffer (PixelBufferDescriptor)
 screen
 +--------------------+
 |                    |                .stride         .alignment
 |                    |         ----------------------->-->
 |                    |         O----------------------+--+   low addresses
 |                    |         |          |           |  |
 |             w      |         |          | .top      |  |
 |       <--------->  |         |          V           |  |
 |       +---------+  |         |     +---------+      |  |
 |       |     ^   |  | ======> |     |         |      |  |
 |   x   |    h|   |  |         |.left|         |      |  |
 +------>|     v   |  |         +---->|         |      |  |
 |       +.........+  |         |     +.........+      |  |
 |            ^       |         |                      |  |
 |          y |       |         +----------------------+--+  high addresses
 O------------+-------+

```

Typically `readPixels` will be called after [render](render.md) and before [endFrame](end-frame.md).

After issuing this method, the callback associated with `buffer` will be invoked on the main thread, indicating that the read-back has completed. Typically, this will happen after multiple calls to [beginFrame](begin-frame.md), [render](render.md), [endFrame](end-frame.md).

It is also possible to use a [Fence](../-fence/index.md) to wait for the read-back.

OpenGL only: if issuing a `readPixels` on a [RenderTarget](../-render-target/index.md) backed by a [Texture](../-texture/index.md) that had data uploaded to it via setImage, the data returned from `readPixels` will be y-flipped with respect to the setImage call.

`readPixels` is intended for debugging and testing. It will impact performance significantly.

#### Parameters

main

| | |
|---|---|
| renderTarget | [RenderTarget](../-render-target/index.md) to read back from |
| xoffset | left offset of the sub-region to read back |
| yoffset | bottom offset of the sub-region to read back |
| width | width of the sub-region to read back |
| height | height of the sub-region to read back |
| buffer | client-side buffer where the read-back will be written <br>The following format are always supported: <br>[RGBA](../-texture/-format/-r-g-b-a/index.md)[RGBA_INTEGER](../-texture/-format/-r-g-b-a_-i-n-t-e-g-e-r/index.md)<br>The following types are always supported: <br>[UBYTE](../-texture/-type/-u-b-y-t-e/index.md)[UINT](../-texture/-type/-u-i-n-t/index.md)[INT](../-texture/-type/-i-n-t/index.md)[FLOAT](../-texture/-type/-f-l-o-a-t/index.md)<br>Other combination of format/type may be supported. If a combination is not supported, this operation may fail silently. Use a DEBUG build to get some logs about the failure. |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |
