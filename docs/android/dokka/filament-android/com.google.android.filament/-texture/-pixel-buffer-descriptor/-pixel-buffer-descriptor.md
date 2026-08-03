//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[PixelBufferDescriptor](index.md)/[PixelBufferDescriptor](-pixel-buffer-descriptor.md)

# PixelBufferDescriptor

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Creates a `PixelBufferDescriptor`

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Pixel [format](../-format/index.md) of the CPU-side image |
| type | Pixel data [type](../-type/index.md) of the CPU-side image |
| alignment | Row-alignment in bytes of the CPU-side image (1 to 8 bytes) |
| left | Left coordinate in pixels of the CPU-side image |
| top | Top coordinate in pixels of the CPU-side image |
| stride | Stride in pixels of the CPU-side image (i.e. distance in pixels to the next row) |
| handler | An [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | A callback executed by `handler` when `storage` is no longer needed. |

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md))

Creates a `PixelBufferDescriptor` with some default values and no callback.

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Pixel [format](../-format/index.md) of the CPU-side image |
| type | Pixel data [type](../-type/index.md) of the CPU-side image |

#### See also

| |
|---|
| [setCallback](set-callback.md) |

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Creates a `PixelBufferDescriptor` with some default values and no callback.

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Pixel [format](../-format/index.md) of the CPU-side image |
| type | Pixel data [type](../-type/index.md) of the CPU-side image |
| alignment | Row-alignment in bytes of the CPU-side image (1 to 8 bytes) |

#### See also

| |
|---|
| [setCallback](set-callback.md) |

[main]\
constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Creates a `PixelBufferDescriptor` with some default values and no callback.

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Pixel [format](../-format/index.md) of the CPU-side image |
| type | Pixel data [type](../-type/index.md) of the CPU-side image |
| alignment | Row-alignment in bytes of the CPU-side image (1 to 8 bytes) |
| left | Left coordinate in pixels of the CPU-side image |
| top | Top coordinate in pixels of the CPU-side image |

#### See also

| |
|---|
| [setCallback](set-callback.md) |

[main]\
constructor(storage: [ByteBuffer](https://developer.android.com/reference/kotlin/java/nio/ByteBuffer.html), format: [Texture.CompressedFormat](../-compressed-format/index.md), compressedSizeInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

#### Parameters

main

| | |
|---|---|
| storage | CPU-side buffer containing the image data to upload into the texture |
| format | Compressed pixel [format](../-compressed-format/index.md) of the CPU-side image |
| compressedSizeInBytes | Size of the compressed data in bytes |

#### See also

| |
|---|
| [setCallback](set-callback.md) |
