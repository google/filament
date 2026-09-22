//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[PixelBufferDescriptor](index.md)/[PixelBufferDescriptor](-pixel-buffer-descriptor.md)

# PixelBufferDescriptor

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: Int, left: Int, top: Int, stride: Int, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Creates a `PixelBufferDescriptor`.

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Pixel [format](../-texture/-format/index.md) of the CPU-side image |
| type | Pixel data [type](../-texture/-type/index.md) of the CPU-side image |
| alignment | Row-alignment in bytes of the CPU-side image (1 to 8 bytes) |
| left | Left coordinate in pixels of the CPU-side image |
| top | Top coordinate in pixels of the CPU-side image |
| stride | Stride in pixels of the CPU-side image |
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html) or Android Handler |
| callback | A callback executed by `handler` when `storage` is no longer needed |

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md))

Creates a `PixelBufferDescriptor` with default alignment (1) and offsets (0), without callback.

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: Int)

Creates a `PixelBufferDescriptor` with specified alignment and default offsets (0), without callback.

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: Int, left: Int, top: Int)

Creates a `PixelBufferDescriptor` with specified alignment and left/top coordinates, without callback.

[main]\
constructor(storage: [ByteBuffer](https://developer.android.com/reference/kotlin/java/nio/ByteBuffer.html), format: [Texture.CompressedType](../-texture/-compressed-type/index.md), compressedSizeInBytes: Int)

Creates a `PixelBufferDescriptor` referencing compressed image data in main memory.

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Compressed pixel [format](../-texture/-compressed-type/index.md) of the CPU-side image |
| compressedSizeInBytes | Size of the compressed data in bytes |
