//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndexBuffer](index.md)/[setBuffer](set-buffer.md)

# setBuffer

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))

Copy-initializes a region of this IndexBuffer from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this IndexBuffer with. |
| buffer | A BufferDescriptor representing the data used to initialize the IndexBuffer. BufferDescriptor points to raw, untyped data that will be interpreted as either 16-bit or 32-bits indices based on the Type of this IndexBuffer. |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Copy-initializes a region of this IndexBuffer from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this IndexBuffer with. |
| buffer | A BufferDescriptor representing the data used to initialize the IndexBuffer. BufferDescriptor points to raw, untyped data that will be interpreted as either 16-bit or 32-bits indices based on the Type of this IndexBuffer. |
| destOffsetInBytes | Offset in *bytes* into the IndexBuffer. Must be multiple of 4. |
| count | number of bytes to copy |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Copy-initializes a region of this IndexBuffer from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this IndexBuffer with. |
| buffer | A BufferDescriptor representing the data used to initialize the IndexBuffer. BufferDescriptor points to raw, untyped data that will be interpreted as either 16-bit or 32-bits indices based on the Type of this IndexBuffer. |
| destOffsetInBytes | Offset in *bytes* into the IndexBuffer. Must be multiple of 4. |
| count | number of bytes to copy |
| handler | handler to dispatch the callback or null for the default handler |
| callback | runnable called upon completion of the operation |
