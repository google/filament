//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setBonesAsQuaternions](set-bones-as-quaternions.md)

# setBonesAsQuaternions

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), quaternions: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Sets the transforms associated with each bone of a Renderable.

#### Parameters

main

| | |
|---|---|
| i | Instance of the Renderable |
| quaternions | A FloatBuffer containing boneCount transforms. Each transform consists of 8 float. float 0 to 3 encode a unit quaternion w+ix+jy+kz stored as x,y,z,w. float 4 to 7 encode a translation stored as x,y,z,1 |
| boneCount | Number of bones to set |
| offset | Index of the first bone to set |
