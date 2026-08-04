//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setCustomEyeProjection](set-custom-eye-projection.md)

# setCustomEyeProjection

[main]\
open fun [setCustomEyeProjection](set-custom-eye-projection.md)(inProjection: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, count: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), inProjectionForCulling: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;, near: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), far: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets a custom projection matrix for each eye.

#### Parameters

main

| | |
|---|---|
| inProjection | An array of projection matrices, one for each eye. Must have at least 16 * count elements. |
| count | Number of eyes to set. |
| inProjectionForCulling | Custom projection matrix for culling, must encompass all eyes. |
| near | Distance to the near plane. |
| far | Distance to the far plane. |
