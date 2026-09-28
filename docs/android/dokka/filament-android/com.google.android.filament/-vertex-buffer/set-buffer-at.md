//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[VertexBuffer](index.md)/[setBufferAt](set-buffer-at.md)

# setBufferAt

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))

copy-initializes the specified buffer from the given buffer data. 

Do not use this if you called enableBufferObjects() on the Builder.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this VertexBuffer with. |
| bufferIndex | Index of the buffer to initialize. Must be between 0 and Builder::bufferCount() - 1. |
| buffer | A BufferDescriptor representing the data used to initialize the buffer at index `bufferIndex`. BufferDescriptor points to raw, untyped data that will be copied as-is into the buffer. |

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

copy-initializes the specified buffer from the given buffer data. 

Do not use this if you called enableBufferObjects() on the Builder.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this VertexBuffer with. |
| bufferIndex | Index of the buffer to initialize. Must be between 0 and Builder::bufferCount() - 1. |
| buffer | A BufferDescriptor representing the data used to initialize the buffer at index `bufferIndex`. BufferDescriptor points to raw, untyped data that will be copied as-is into the buffer. |
| destOffsetInBytes | Offset in *bytes* into the buffer at index `bufferIndex` of this vertex buffer set. Must be multiple of 4. |
| count | number of bytes to copy |

[main]\
open fun [setBufferAt](set-buffer-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

copy-initializes the specified buffer from the given buffer data. 

Do not use this if you called enableBufferObjects() on the Builder.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this VertexBuffer with. |
| bufferIndex | Index of the buffer to initialize. Must be between 0 and Builder::bufferCount() - 1. |
| buffer | A BufferDescriptor representing the data used to initialize the buffer at index `bufferIndex`. BufferDescriptor points to raw, untyped data that will be copied as-is into the buffer. |
| destOffsetInBytes | Offset in *bytes* into the buffer at index `bufferIndex` of this vertex buffer set. Must be multiple of 4. |
| count | number of bytes to copy |
| handler | handler to dispatch the callback or null for the default handler |
| callback | runnable called upon completion of the operation |
