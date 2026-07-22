//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[curves](curves.md)

# curves

[main]\
open fun [curves](curves.md)(shadowGamma: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, midPoint: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, highlightScale: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [ColorGrading.Builder](index.md)

Applies a curve to each RGB channel of the image. Each curve is defined by 3 values: a gamma value applied to the shadows only, a mid-point indicating where shadows stop and highlights start, and a scale factor for the highlights. The gamma and mid-point must be strictly positive values. If they are not, they will be clamped to a small positive value. The scale can be any negative of positive value. Curves are applied in linear space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| shadowGamma | Power value to apply to the shadows, must be strictly positive |
| midPoint | Mid-point defining where shadows stop and highlights start, must be strictly positive |
| highlightScale | Scale factor for the highlights, can be any negative or positive value |
