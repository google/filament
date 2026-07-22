//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[SkinningBuffer](../index.md)/[Builder](index.md)/[boneCount](bone-count.md)

# boneCount

[main]\
open fun [boneCount](bone-count.md)(boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [SkinningBuffer.Builder](index.md)

Size of the skinning buffer in bones. 

Due to limitation in the GLSL, the SkinningBuffer must always by a multiple of 256, this adjustment is done automatically, but can cause some memory overhead. This memory overhead can be mitigated by using the same [SkinningBuffer](../index.md) to store the bone information for multiple RenderPrimitives.

#### Return

this `Builder` object for chaining calls

#### Parameters

main

| | |
|---|---|
| boneCount | Number of bones the skinning buffer can hold. |
