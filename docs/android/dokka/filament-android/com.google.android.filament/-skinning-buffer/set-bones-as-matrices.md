//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SkinningBuffer](index.md)/[setBonesAsMatrices](set-bones-as-matrices.md)

# setBonesAsMatrices

[main]\
open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), matrices: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [offset, offset + boneCount).

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) instance |
| matrices | A [FloatBuffer](https://developer.android.com/reference/kotlin/java/nio/FloatBuffer.html) containing boneCount 4x4 packed matrices (i.e. 16 floats each matrix and no gap between matrices) |
| boneCount | Number of bones to set |
| offset | Index of the first bone to set |
