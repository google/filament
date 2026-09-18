//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[PixelBufferDescriptor](index.md)

# PixelBufferDescriptor

open class [PixelBufferDescriptor](index.md)

A descriptor to an image in main memory, typically used to transfer image data from the CPU to the GPU. 

A PixelBufferDescriptor owns the memory buffer it references, therefore PixelBufferDescriptor cannot be copied, but can be moved.

PixelBufferDescriptor releases ownership of the memory-buffer when it's destroyed.

#### Inheritors

| |
|---|
| [PixelBufferDescriptor](../-texture/-pixel-buffer-descriptor/index.md) |

## Constructors

| | |
|---|---|
| [PixelBufferDescriptor](-pixel-buffer-descriptor.md) | [main]<br>constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Creates a `PixelBufferDescriptor`.<br>constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md))<br>Creates a `PixelBufferDescriptor` with default alignment (1) and offsets (0), without callback.<br>constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Creates a `PixelBufferDescriptor` with specified alignment and default offsets (0), without callback.<br>constructor(storage: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), left: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), top: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Creates a `PixelBufferDescriptor` with specified alignment and left/top coordinates, without callback.<br>constructor(storage: [ByteBuffer](https://developer.android.com/reference/kotlin/java/nio/ByteBuffer.html), format: [Texture.CompressedType](../-texture/-compressed-type/index.md), compressedSizeInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Creates a `PixelBufferDescriptor` referencing compressed image data in main memory. |

## Properties

| Name | Summary |
|---|---|
| [alignment](alignment.md) | [main]<br>open var [alignment](alignment.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [callback](callback.md) | [main]<br>open var [callback](callback.md): [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html) |
| [compressedFormat](compressed-format.md) | [main]<br>open var [compressedFormat](compressed-format.md): [Texture.CompressedType](../-texture/-compressed-type/index.md) |
| [compressedSizeInBytes](compressed-size-in-bytes.md) | [main]<br>open var [compressedSizeInBytes](compressed-size-in-bytes.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [format](format.md) | [main]<br>open var [format](format.md): [Texture.Format](../-texture/-format/index.md) |
| [handler](handler.md) | [main]<br>open var [handler](handler.md): [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html) |
| [left](left.md) | [main]<br>open var [left](left.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [storage](storage.md) | [main]<br>open var [storage](storage.md): [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) |
| [stride](stride.md) | [main]<br>open var [stride](stride.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [top](top.md) | [main]<br>open var [top](top.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [type](type.md) | [main]<br>open var [type](type.md): [Texture.Type](../-texture/-type/index.md) |

## Functions

| Name | Summary |
|---|---|
| [computeDataSize](compute-data-size.md) | [main]<br>open fun [computeDataSize](compute-data-size.md)(format: [Texture.Format](../-texture/-format/index.md), type: [Texture.Type](../-texture/-type/index.md), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Helper to calculate the buffer size (in bytes) needed for given parameters. |
| [setCallback](set-callback.md) | [main]<br>open fun [setCallback](set-callback.md)(handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Set or replace the callback called when the CPU-side data is no longer needed. |
