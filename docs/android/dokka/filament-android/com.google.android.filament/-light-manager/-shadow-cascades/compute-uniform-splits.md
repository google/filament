//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowCascades](index.md)/[computeUniformSplits](compute-uniform-splits.md)

# computeUniformSplits

[main]\
open fun [computeUniformSplits](compute-uniform-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a uniform split scheme.

#### Parameters

main

| | |
|---|---|
| splitPositions | a float array of at least size (cascades - 1) to write the split positions into |
| cascades | the number of shadow cascades, at most 4 |
