//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[BufferObject](index.md)

# BufferObject

open class [BufferObject](index.md)

A generic GPU buffer containing data. Usage of this BufferObject is optional. For simple use cases it is not necessary. It is useful only when you need to share data between multiple VertexBuffer instances. It also allows you to efficiently swap-out the buffers in VertexBuffer. NOTE: For now this is only used for vertex data, but in the future we may use it for other things (e.g. compute).

#### See also

| |
|---|
| [VertexBuffer](../-vertex-buffer/index.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getByteCount](get-byte-count.md) | [main]<br>open fun [getByteCount](get-byte-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the size of this `BufferObject` in elements. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setBuffer](set-buffer.md) | [main]<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))<br>Asynchronously copy-initializes this `BufferObject` from the data provided.<br>[main]<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Asynchronously copy-initializes a region of this `BufferObject` from the data provided. |
