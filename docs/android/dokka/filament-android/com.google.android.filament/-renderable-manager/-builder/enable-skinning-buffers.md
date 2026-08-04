//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[enableSkinningBuffers](enable-skinning-buffers.md)

# enableSkinningBuffers

[main]\
open fun [enableSkinningBuffers](enable-skinning-buffers.md)(enabled: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)): [RenderableManager.Builder](index.md)

Allows bones to be swapped out and shared using SkinningBuffer. If skinning buffer mode is enabled, clients must call #setSkinningBuffer() rather than #setBonesAsQuaternions(). This allows sharing of data between renderables.

#### Parameters

main

| | |
|---|---|
| enabled | If true, enables buffer object mode. False by default. |
