//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [attribute](attribute.md) | [main]<br>open fun [attribute](attribute.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), attributeType: [VertexBuffer.AttributeType](../-attribute-type/index.md)): [VertexBuffer.Builder](index.md)<br>open fun [attribute](attribute.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), attributeType: [VertexBuffer.AttributeType](../-attribute-type/index.md), byteOffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), byteStride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)<br>Sets up an attribute for this vertex buffer set. |
| [bufferCount](buffer-count.md) | [main]<br>open fun [bufferCount](buffer-count.md)(bufferCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)<br>Defines how many buffers will be created in this vertex buffer set. |
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [VertexBuffer](../index.md)<br>Creates the `VertexBuffer` object and returns a pointer to it. |
| [enableBufferObjects](enable-buffer-objects.md) | [main]<br>open fun [enableBufferObjects](enable-buffer-objects.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [VertexBuffer.Builder](index.md)<br>Allows buffers to be swapped out and shared using BufferObject. |
| [normalized](normalized.md) | [main]<br>open fun [normalized](normalized.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md)): [VertexBuffer.Builder](index.md)<br>open fun [normalized](normalized.md)(attribute: [VertexBuffer.VertexAttribute](../-vertex-attribute/index.md), enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [VertexBuffer.Builder](index.md)<br>Sets whether a given attribute should be normalized. |
| [vertexCount](vertex-count.md) | [main]<br>open fun [vertexCount](vertex-count.md)(vertexCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [VertexBuffer.Builder](index.md)<br>Size of each buffer in this set, expressed in in number of vertices. |
