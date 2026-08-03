//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[BufferObject](index.md)/[setBuffer](set-buffer.md)

# setBuffer

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))

Asynchronously copy-initializes this `BufferObject` from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `BufferObject` with |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) with the data used to initialize the `BufferObject`. |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Asynchronously copy-initializes a region of this `BufferObject` from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `BufferObject` with |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) with the data used to initialize the `BufferObject`. |
| destOffsetInBytes | offset in *bytes* into the `BufferObject` |
| count | number of bytes to consume, defaults to `buffer.remaining()` |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously copy-initializes a region of this `BufferObject` from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `BufferObject` with |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) with the data used to initialize the `BufferObject`. |
| destOffsetInBytes | offset in *bytes* into the `BufferObject` |
| count | number of bytes to consume, defaults to `buffer.remaining()` |
| handler | an [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | a callback executed by `handler` when `buffer`is no longer needed. |
