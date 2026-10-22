//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[VertexBuffer](index.md)

# VertexBuffer

open class [VertexBuffer](index.md)

Holds a set of buffers that define the geometry of a Renderable. 

The geometry of the Renderable itself is defined by a set of vertex attributes such as position, color, normals, tangents, etc...

There is no need to have a 1-to-1 mapping between attributes and buffer. A buffer can hold the data of several attributes -- attributes are then referred as being &quot;interleaved&quot;.

The buffers themselves are GPU resources, therefore mutating their data can be relatively slow. For this reason, it is best to separate the constant data from the dynamic data into multiple buffers.

It is possible, and even encouraged, to use a single vertex buffer for several Renderables.

#### See also

| |
|---|
| [IndexBuffer](../-index-buffer/index.md) |
| [RenderableManager](../-renderable-manager/index.md) |

## Types

| Name | Summary |
|---|---|
| [AsyncCallStatus](-async-call-status/index.md) | [main]<br>enum [AsyncCallStatus](-async-call-status/index.md)<br>Outcome of an asynchronous operation, reported to its completion callback. |
| [AttributeType](-attribute-type/index.md) | [main]<br>enum [AttributeType](-attribute-type/index.md)<br>Supported element types |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |
| [VertexAttribute](-vertex-attribute/index.md) | [main]<br>enum [VertexAttribute](-vertex-attribute/index.md)<br>Vertex attribute types |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getVertexCount](get-vertex-count.md) | [main]<br>open fun [getVertexCount](get-vertex-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the vertex count. |
| [isCreationComplete](is-creation-complete.md) | [main]<br>open fun [isCreationComplete](is-creation-complete.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>This non-blocking method checks if the resource has finished creation *successfully*. |
| [setBufferAt](set-buffer-at.md) | [main]<br>open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))<br>open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>copy-initializes the specified buffer from the given buffer data. |
| [setBufferObjectAt](set-buffer-object-at.md) | [main]<br>open fun [setBufferObjectAt](set-buffer-object-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bufferObject: [BufferObject](../-buffer-object/index.md))<br>Swaps in the given buffer object. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [VertexBuffer](index.md) |
