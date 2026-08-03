//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MorphTargetBuffer](index.md)/[setTangentsAt](set-tangents-at.md)

# setTangentsAt

[main]\
open fun [setTangentsAt](set-tangents-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), tangents: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Short](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-short/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates tangents for the given morph target. These quaternions must be represented as signed shorts, where real numbers in the [-1,+1] range multiplied by 32767.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) instance |
| targetIndex | The index of morph target to be updated |
| tangents | An array with at least &quot;count*4&quot; shorts |
| count | number of short4 quaternions in tangents |
