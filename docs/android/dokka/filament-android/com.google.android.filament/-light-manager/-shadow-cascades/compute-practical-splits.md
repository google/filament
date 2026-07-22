//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowCascades](index.md)/[computePracticalSplits](compute-practical-splits.md)

# computePracticalSplits

[main]\
open fun [computePracticalSplits](compute-practical-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), near: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), far: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), lambda: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a practical split scheme. 

 The practical split scheme uses uses a lambda value to interpolate between the logrithmic and uniform split schemes. Start with a lambda value of 0.5f and adjust for your scene. 

 See: Zhang et al 2006, &quot;Parallel-split shadow maps for large-scale virtual environments&quot;

#### Parameters

main

| | |
|---|---|
| splitPositions | a float array of at least size (cascades - 1) to write the split positions into |
| cascades | the number of shadow cascades, at most 4 |
| near | the camera near plane |
| far | the camera far plane |
| lambda | a float in the range [0, 1] that interpolates between log and uniform split schemes |
