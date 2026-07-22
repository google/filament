//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndexBuffer](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Types

| Name | Summary |
|---|---|
| [IndexType](-index-type/index.md) | [main]<br>enum [IndexType](-index-type/index.md)<br>Type of the index buffer. |

## Functions

| Name | Summary |
|---|---|
| [bufferType](buffer-type.md) | [main]<br>open fun [bufferType](buffer-type.md)(indexType: [IndexBuffer.Builder.IndexType](-index-type/index.md)): [IndexBuffer.Builder](index.md)<br>Type of the index buffer, 16-bit or 32-bit. |
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndexBuffer](../index.md)<br>Creates and returns the `IndexBuffer` object. |
| [indexCount](index-count.md) | [main]<br>open fun [indexCount](index-count.md)(indexCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [IndexBuffer.Builder](index.md)<br>Size of the index buffer in elements. |
