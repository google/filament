//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[PixelBufferDescriptor](index.md)

# PixelBufferDescriptor

[main]\
open class [~~PixelBufferDescriptor~~](index.md) : [PixelBufferDescriptor](../../-pixel-buffer-descriptor/index.md)---

### Deprecated

---

#### Deprecated

Use [com.google.android.filament.PixelBufferDescriptor](../../-pixel-buffer-descriptor/index.md) instead.

## Constructors

| | |
|---|---|
| [PixelBufferDescriptor](-pixel-buffer-descriptor.md) | [main]<br>constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md))constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))constructor(storage: [ByteBuffer](https://developer.android.com/reference/kotlin/java/nio/ByteBuffer.html), compressedFormat: [Texture.CompressedType](../-compressed-type/index.md), compressedSizeInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))constructor(storage: [ByteBuffer](https://developer.android.com/reference/kotlin/java/nio/ByteBuffer.html), compressedFormat: [Texture.CompressedType](../-compressed-type/index.md), compressedSizeInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [alignment](../../-pixel-buffer-descriptor/alignment.md) | [main]<br>open var [alignment](../../-pixel-buffer-descriptor/alignment.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [callback](../../-pixel-buffer-descriptor/callback.md) | [main]<br>open var [callback](../../-pixel-buffer-descriptor/callback.md): [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html) |
| [compressedFormat](../../-pixel-buffer-descriptor/compressed-format.md) | [main]<br>open var [compressedFormat](../../-pixel-buffer-descriptor/compressed-format.md): [Texture.CompressedType](../-compressed-type/index.md) |
| [compressedSizeInBytes](../../-pixel-buffer-descriptor/compressed-size-in-bytes.md) | [main]<br>open var [compressedSizeInBytes](../../-pixel-buffer-descriptor/compressed-size-in-bytes.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [format](../../-pixel-buffer-descriptor/format.md) | [main]<br>open var [format](../../-pixel-buffer-descriptor/format.md): [Texture.Format](../-format/index.md) |
| [handler](../../-pixel-buffer-descriptor/handler.md) | [main]<br>open var [handler](../../-pixel-buffer-descriptor/handler.md): [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html) |
| [left](../../-pixel-buffer-descriptor/left.md) | [main]<br>open var [left](../../-pixel-buffer-descriptor/left.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [storage](../../-pixel-buffer-descriptor/storage.md) | [main]<br>open var [storage](../../-pixel-buffer-descriptor/storage.md): [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) |
| [stride](../../-pixel-buffer-descriptor/stride.md) | [main]<br>open var [stride](../../-pixel-buffer-descriptor/stride.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [top](../../-pixel-buffer-descriptor/top.md) | [main]<br>open var [top](../../-pixel-buffer-descriptor/top.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [type](../../-pixel-buffer-descriptor/type.md) | [main]<br>open var [type](../../-pixel-buffer-descriptor/type.md): [Texture.Type](../-type/index.md) |

## Functions

| Name | Summary |
|---|---|
| [computeDataSize](../../-pixel-buffer-descriptor/compute-data-size.md) | [main]<br>open fun [computeDataSize](../../-pixel-buffer-descriptor/compute-data-size.md)(format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Helper to calculate the buffer size (in bytes) needed for given parameters. |
| [setCallback](../../-pixel-buffer-descriptor/set-callback.md) | [main]<br>open fun [setCallback](../../-pixel-buffer-descriptor/set-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Set or replace the callback called when the CPU-side data is no longer needed. |
