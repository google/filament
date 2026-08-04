//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setMorphWeights](set-morph-weights.md)

# setMorphWeights

[main]\
open fun [setMorphWeights](set-morph-weights.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), weights: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, offset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Updates the vertex morphing weights on a renderable, all zeroes by default. 

The renderable must be built with morphing enabled. In legacy morphing mode, only the first 4 weights are considered.

#### See also

| |
|---|
| com.google.android.filament.RenderableManager.Builder |
