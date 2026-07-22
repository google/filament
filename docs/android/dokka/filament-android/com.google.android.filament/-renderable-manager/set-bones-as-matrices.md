//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setBonesAsMatrices](set-bones-as-matrices.md)

# setBonesAsMatrices

[main]\
open fun [setBonesAsMatrices](set-bones-as-matrices.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), matrices: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Sets the transforms associated with each bone of a Renderable.

#### Parameters

main

| | |
|---|---|
| i | Instance of the Renderable |
| matrices | A FloatBuffer containing boneCount 4x4 packed matrices (i.e. 16 floats each matrix and no gap between matrices) |
| boneCount | Number of bones to set |
| offset | Index of the first bone to set |
