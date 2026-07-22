//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[VertexBuffer](index.md)/[setBufferAt](set-buffer-at.md)

# setBufferAt

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))

Asynchronously copy-initializes the specified buffer from the given buffer data.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `VertexBuffer` with |
| bufferIndex | index of the buffer to initialize. Must be between 0 and bufferCount() - 1. |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) representing the data used to initialize the `VertexBuffer` at index `bufferIndex`. `buffer` should contain raw, untyped data that will be copied as-is into the buffer. |

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Asynchronously copy-initializes a region of the specified buffer from the given buffer data.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `VertexBuffer` with |
| bufferIndex | index of the buffer to initialize. Must be between 0 and bufferCount() - 1. |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) representing the data used to initialize the `VertexBuffer` at index `bufferIndex`. `buffer` should contain raw, untyped data that will be copied as-is into the buffer. |
| destOffsetInBytes | offset in *bytes* into the buffer at index `bufferIndex` of this vertex buffer set. |

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously copy-initializes a region of the specified buffer from the given buffer data.

#### Parameters

main

| | |
|---|---|
| engine | reference to the [Engine](../-engine/index.md) to associate this `VertexBuffer` with |
| bufferIndex | index of the buffer to initialize. Must be between 0 and bufferCount() - 1. |
| buffer | a CPU-side [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html) representing the data used to initialize the `VertexBuffer` at index `bufferIndex`. `buffer` should contain raw, untyped data that will be copied as-is into the buffer. |
| destOffsetInBytes | offset in *bytes* into the buffer at index `bufferIndex` of this vertex buffer set. |
| handler | an [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html). On Android this can also be a Handler. |
| callback | a callback executed by `handler` when `buffer`is no longer needed. |
