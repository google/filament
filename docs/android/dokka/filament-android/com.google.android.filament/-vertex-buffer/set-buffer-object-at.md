//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[VertexBuffer](index.md)/[setBufferObjectAt](set-buffer-object-at.md)

# setBufferObjectAt

[main]\
open fun [setBufferObjectAt](set-buffer-object-at.md)(engine: [Engine](../-engine/index.md), bufferIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), bufferObject: [BufferObject](../-buffer-object/index.md))

Swaps in the given buffer object. To use this, you must first call enableBufferObjects() on the Builder.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this VertexBuffer with. |
| bufferIndex | Index of the buffer to initialize. Must be between 0 and Builder::bufferCount() - 1. |
| bufferObject | The handle to the GPU data that will be used in this buffer slot. |
