//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MorphTargetBuffer](index.md)/[setPositionsAt](set-positions-at.md)

# setPositionsAt

[main]\
open fun [setPositionsAt](set-positions-at.md)(engine: [Engine](../-engine/index.md), targetIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), positions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates float4 positions for the given morph target.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) instance |
| targetIndex | The index of morph target to be updated |
| positions | An array with at least count*4 floats |
| count | Number of float4 vectors in positions to be consumed |
