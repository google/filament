//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[IndexBuffer](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [IndexBuffer](../index.md)

Creates the IndexBuffer object and returns a pointer to it. 

After creation, the index buffer is uninitialized. Use IndexBuffer::setBuffer() to initialize the IndexBuffer.

#### Return

pointer to the newly created object.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this IndexBuffer with. |

#### See also

| |
|---|
| com.google.android.filament.IndexBuffer |
