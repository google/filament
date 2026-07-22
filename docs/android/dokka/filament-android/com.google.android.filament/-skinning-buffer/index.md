//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[SkinningBuffer](index.md)

# SkinningBuffer

[main]\
open class [SkinningBuffer](index.md)

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md) |

## Functions

| Name | Summary |
|---|---|
| [getBoneCount](get-bone-count.md) | [main]<br>open fun [getBoneCount](get-bone-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setBonesAsMatrices](set-bones-as-matrices.md) | [main]<br>open fun [setBonesAsMatrices](set-bones-as-matrices.md)(engine: [Engine](../-engine/index.md), matrices: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Updates the bone transforms in the range [offset, offset + boneCount). |
| [setBonesAsQuaternions](set-bones-as-quaternions.md) | [main]<br>open fun [setBonesAsQuaternions](set-bones-as-quaternions.md)(engine: [Engine](../-engine/index.md), quaternions: [Buffer](https://developer.android.com/reference/kotlin/java/nio/Buffer.html), boneCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Updates the bone transforms in the range [offset, offset + boneCount). |
