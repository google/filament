//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[BufferObject](index.md)/[setBuffer](set-buffer.md)

# setBuffer

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html))

Asynchronously copy-initializes a region of this BufferObject from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this BufferObject. |
| buffer | A BufferDescriptor representing the data used to initialize the BufferObject. |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: Int, count: Int)

Asynchronously copy-initializes a region of this BufferObject from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this BufferObject. |
| buffer | A BufferDescriptor representing the data used to initialize the BufferObject. |
| destOffsetInBytes | Offset in bytes into the BufferObject. Must be multiple of 4. |
| count | number of bytes to copy |

[main]\
open fun [setBuffer](set-buffer.md)(engine: [Engine](../-engine/index.md), buffer: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), destOffsetInBytes: Int, count: Int, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Asynchronously copy-initializes a region of this BufferObject from the data provided.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this BufferObject. |
| buffer | A BufferDescriptor representing the data used to initialize the BufferObject. |
| destOffsetInBytes | Offset in bytes into the BufferObject. Must be multiple of 4. |
| count | number of bytes to copy |
| handler | handler to dispatch the callback or null for the default handler |
| callback | runnable called upon completion of the operation |
