//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[ShadowCascades](index.md)

# ShadowCascades

[main]\
open class [ShadowCascades](index.md)

## Constructors

| | |
|---|---|
| [ShadowCascades](-shadow-cascades.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [computeLogSplits](compute-log-splits.md) | [main]<br>open fun [computeLogSplits](compute-log-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), near: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), far: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a logarithmic split scheme. |
| [computePracticalSplits](compute-practical-splits.md) | [main]<br>open fun [computePracticalSplits](compute-practical-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), near: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), far: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), lambda: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a practical split scheme. |
| [computeUniformSplits](compute-uniform-splits.md) | [main]<br>open fun [computeUniformSplits](compute-uniform-splits.md)(splitPositions: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, cascades: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Utility method to compute [cascadeSplitPositions](../-shadow-options/cascade-split-positions.md) according to a uniform split scheme. |
