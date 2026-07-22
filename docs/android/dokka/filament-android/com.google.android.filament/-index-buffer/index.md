//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndexBuffer](index.md)

# IndexBuffer

open class [IndexBuffer](index.md)

A buffer containing vertex indices into a `VertexBuffer`. Indices can be 16 or 32 bit. The buffer itself is a GPU resource, therefore mutating the data can be relatively slow. Typically these buffers are constant. It is possible, and even encouraged, to use a single index buffer for several `Renderables`.

#### See also

| |
|---|
| [VertexBuffer](../-vertex-buffer/index.md) |
| [RenderableManager](../-renderable-manager/index.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getIndexCount](get-index-count.md) | [main]<br>open fun [getIndexCount](get-index-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the size of this `IndexBuffer` in elements. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setBuffer](set-buffer.md) | [main]<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))<br>Asynchronously copy-initializes this `IndexBuffer` from the data provided.<br>[main]<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Asynchronously copy-initializes a region of this `IndexBuffer` from the data provided. |
