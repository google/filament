//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[BufferObject](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [BufferObject](../index.md)

Creates the BufferObject and returns a pointer to it. 

After creation, the buffer object is uninitialized. Use BufferObject::setBuffer() to initialize it.

#### Return

pointer to the newly created object

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this BufferObject with. |

#### See also

| |
|---|
| com.google.android.filament.IndexBuffer |
