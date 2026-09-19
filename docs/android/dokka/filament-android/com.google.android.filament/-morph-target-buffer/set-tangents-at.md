//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MorphTargetBuffer](index.md)/[setTangentsAt](set-tangents-at.md)

# setTangentsAt

[main]\
open fun [setTangentsAt](set-tangents-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), tangents: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Short](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-short/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates tangents for the given morph target. 

This method can only be called if the MorphTargetBuffer was built with `withTangents(true)`. These quaternions must be represented as signed shorts, where real numbers in the [-1,+1] range multiplied by 32767.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this MorphTargetBuffer. |
| targetIndex | the index of morph target to be updated. |
| tangents | pointer to at least &quot;count&quot; tangents |
| count | number of short4 quaternions in tangents |

[main]\
open fun [setTangentsAt](set-tangents-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), tangents: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Short](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-short/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates tangents for the given morph target. 

This method can only be called if the MorphTargetBuffer was built with `withTangents(true)`. These quaternions must be represented as signed shorts, where real numbers in the [-1,+1] range multiplied by 32767.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine associated with this MorphTargetBuffer. |
| targetIndex | the index of morph target to be updated. |
| tangents | pointer to at least &quot;count&quot; tangents |
| count | number of short4 quaternions in tangents |
| offset | offset into the target buffer, expressed as a number of short4 vectors |
