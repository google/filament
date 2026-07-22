//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SkinningBuffer](index.md)/[setBonesAsQuaternions](set-bones-as-quaternions.md)

# setBonesAsQuaternions

[main]\
open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), quaternions: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the bone transforms in the range [offset, offset + boneCount).

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) instance |
| quaternions | A [FloatBuffer](https://developer.android.com/reference/kotlin/java/nio/FloatBuffer.html) containing boneCount transforms. Each transform consists of 8 float. float 0 to 3 encode a unit quaternion `w+ix+jy+kz` stored as `x,y,z,w`. float 4 to 7 encode a translation stored as `x,y,z,1`. |
| boneCount | Number of bones to set |
| offset | Index of the first bone to set |
