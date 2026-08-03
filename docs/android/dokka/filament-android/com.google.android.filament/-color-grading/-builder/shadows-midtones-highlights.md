//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[shadowsMidtonesHighlights](shadows-midtones-highlights.md)

# shadowsMidtonesHighlights

[main]\
open fun [shadowsMidtonesHighlights](shadows-midtones-highlights.md)(shadows: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, midtones: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, highlights: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, ranges: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [ColorGrading.Builder](index.md)

Adjusts the colors separately in 3 distinct tonal ranges or zones: shadows, mid-tones, and highlights. The tonal zones are by the ranges parameter: the x and y components define the beginning and end of the transition from shadows to mid-tones, and the z and w components define the beginning and end of the transition from mid-tones to highlights. A smooth transition is applied between the zones which means for instance that the correction color of the shadows range will partially apply to the mid-tones, and the other way around. This ensure smooth visual transitions in the final image. Each correction color is defined as a linear RGB color and a weight. The weight is a value (which may be positive or negative) that is added to the linear RGB color before mixing. This can be used to darken or brighten the selected tonal range. Shadows/mid-tones/highlights adjustment are performed linear space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| shadows | Linear RGB color (.rgb) and weight (.w) to apply to the shadows |
| midtones | Linear RGB color (.rgb) and weight (.w) to apply to the mid-tones |
| highlights | Linear RGB color (.rgb) and weight (.w) to apply to the highlights |
| ranges | Range of the shadows (x and y), and range of the highlights (z and w) |
