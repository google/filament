//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[readPixels](read-pixels.md)

# readPixels

[main]\
open fun [readPixels](read-pixels.md)(xoffset: Int, yoffset: Int, width: Int, height: Int, buffer: [PixelBufferDescriptor](../-pixel-buffer-descriptor/index.md))

Reads back the content of the SwapChain associated with this Renderer.

#### Parameters

main

| | |
|---|---|
| xoffset | Left offset of the sub-region to read back. |
| yoffset | Bottom offset of the sub-region to read back. |
| width | Width of the sub-region to read back. |
| height | Height of the sub-region to read back. |
| buffer | Client-side buffer where the read-back will be written.<br>The following formats are always supported:<br>- PixelBufferDescriptor::PixelDataFormat::RGBA - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER The following types are always supported: - PixelBufferDescriptor::PixelDataType::UBYTE - PixelBufferDescriptor::PixelDataType::UINT - PixelBufferDescriptor::PixelDataType::INT - PixelBufferDescriptor::PixelDataType::FLOAT Other combinations of format/type may be supported. If a combination is not supported, this operation may fail silently. Use a DEBUG build to get some logs about the failure. Framebuffer as seen on User buffer (PixelBufferDescriptor&) screen +--------------------+<br>| | | | |---|---|---| |  |  |  | |  | O----------------------+--+ low addresses | |  |  |  |  |  | | w |  |  | .top |  | | <---------> |  | V |  | | +---------+ |  | +---------+ |  | |  | ^ |  | ======> |  |  |  |  | | x | h |  |  |  | .left |  |  |  | | +------> | v |  | +----> |  |  |  | | +.........+ |  | +.........+ |  | | ^ |  |  |  | | y |  | +----------------------+--+ high addresses | | O------------+-------+ |<br>readPixels() must be called within a frame, meaning after beginFrame() and before endFrame(). Typically, readPixels() will be called after render().<br>After issuing this method, the callback associated with `buffer` will be invoked on the main thread, indicating that the read-back has completed. Typically, this will happen after multiple calls to beginFrame(), render(), endFrame().<br>It is also possible to use a Fence to wait for the read-back.<br>@remark readPixels() is intended for debugging and testing. It will impact performance significantly. |

[main]\
open fun [readPixels](read-pixels.md)(renderTarget: [RenderTarget](../-render-target/index.md), xoffset: Int, yoffset: Int, width: Int, height: Int, buffer: [PixelBufferDescriptor](../-pixel-buffer-descriptor/index.md))

Reads back the content of the provided RenderTarget.

#### Parameters

main

| | |
|---|---|
| renderTarget | RenderTarget to read back from. |
| xoffset | Left offset of the sub-region to read back. |
| yoffset | Bottom offset of the sub-region to read back. |
| width | Width of the sub-region to read back. |
| height | Height of the sub-region to read back. |
| buffer | Client-side buffer where the read-back will be written.<br>The following formats are always supported:<br>- PixelBufferDescriptor::PixelDataFormat::RGBA - PixelBufferDescriptor::PixelDataFormat::RGBA_INTEGER The following types are always supported: - PixelBufferDescriptor::PixelDataType::UBYTE - PixelBufferDescriptor::PixelDataType::UINT - PixelBufferDescriptor::PixelDataType::INT - PixelBufferDescriptor::PixelDataType::FLOAT Other combinations of format/type may be supported. If a combination is not supported, this operation may fail silently. Use a DEBUG build to get some logs about the failure. Framebuffer as seen on User buffer (PixelBufferDescriptor&) screen +--------------------+<br>| | | | |---|---|---| |  |  |  | |  | O----------------------+--+ low addresses | |  |  |  |  |  | | w |  |  | .top |  | | <---------> |  | V |  | | +---------+ |  | +---------+ |  | |  | ^ |  | ======> |  |  |  |  | | x | h |  |  |  | .left |  |  |  | | +------> | v |  | +----> |  |  |  | | +.........+ |  | +.........+ |  | | ^ |  |  |  | | y |  | +----------------------+--+ high addresses | | O------------+-------+ |<br>Typically readPixels() will be called after render() and before endFrame().<br>After issuing this method, the callback associated with `buffer` will be invoked on the main thread, indicating that the read-back has completed. Typically, this will happen after multiple calls to beginFrame(), render(), endFrame().<br>It is also possible to use a Fence to wait for the read-back.<br>OpenGL only: if issuing a readPixels on a RenderTarget backed by a Texture that had data uploaded to it via setImage, the data returned from readPixels will be y-flipped with respect to the setImage call.<br>Note: the texture that backs the COLOR attachment for `renderTarget` must have TextureUsage::BLIT_SRC as part of its usage.<br>@remark readPixels() is intended for debugging and testing. It will impact performance significantly. |
