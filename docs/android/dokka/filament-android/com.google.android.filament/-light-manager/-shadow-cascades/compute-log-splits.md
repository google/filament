//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowCascades](index.md)/[computeLogSplits](compute-log-splits.md)

# computeLogSplits

[main]\
open fun [computeLogSplits](compute-log-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), near: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), far: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a logarithmic split scheme.

#### Parameters

main

| | |
|---|---|
| splitPositions | a float array of at least size (cascades - 1) to write the split positions into |
| cascades | the number of shadow cascades, at most 4 |
| near | the camera near plane |
| far | the camera far plane |
