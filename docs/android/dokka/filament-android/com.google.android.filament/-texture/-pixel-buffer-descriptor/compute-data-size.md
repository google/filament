//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[PixelBufferDescriptor](index.md)/[computeDataSize](compute-data-size.md)

# computeDataSize

[main]\
open fun [computeDataSize](compute-data-size.md)(format: [Texture.Format](../-format/index.md), type: [Texture.Type](../-type/index.md), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Helper to calculate the buffer size (in byte) needed for given parameters

#### Return

Size of the buffer in bytes.

#### Parameters

main

| | |
|---|---|
| format | Pixel data format. |
| type | Pixel data type. Can't be [COMPRESSED](../-type/-c-o-m-p-r-e-s-s-e-d/index.md). |
| stride | Stride in pixels. |
| height | Height in pixels. |
| alignment | Alignment in bytes. |
