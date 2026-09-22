//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndexBuffer](index.md)

# IndexBuffer

open class [IndexBuffer](index.md)

A buffer containing vertex indices into a VertexBuffer. 

Indices can be 16 or 32 bit. The buffer itself is a GPU resource, therefore mutating the data can be relatively slow. Typically these buffers are constant.

It is possible, and even encouraged, to use a single index buffer for several Renderables.

#### See also

| |
|---|
| [VertexBuffer](../-vertex-buffer/index.md) |
| [RenderableManager](../-renderable-manager/index.md) |

## Types

| Name | Summary |
|---|---|
| [AsyncCallStatus](-async-call-status/index.md) | [main]<br>enum [AsyncCallStatus](-async-call-status/index.md)<br>Outcome of an asynchronous operation, reported to its completion callback. |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |
| [IndexType](-index-type/index.md) | [main]<br>enum [IndexType](-index-type/index.md)<br>Type of the index buffer |

## Functions

| Name | Summary |
|---|---|
| [getIndexCount](get-index-count.md) | [main]<br>open fun [getIndexCount](get-index-count.md)(): Int<br>Returns the size of this IndexBuffer in elements. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [isCreationComplete](is-creation-complete.md) | [main]<br>open fun [isCreationComplete](is-creation-complete.md)(): Boolean<br>This non-blocking method checks if the resource has finished creation *successfully*. |
| [setBuffer](set-buffer.md) | [main]<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: Int, count: Int)<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: Int, count: Int, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Copy-initializes a region of this IndexBuffer from the data provided. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [IndexBuffer](index.md) |
