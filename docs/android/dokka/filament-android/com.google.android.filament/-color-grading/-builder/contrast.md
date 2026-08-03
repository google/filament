//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[contrast](contrast.md)

# contrast

[main]\
open fun [contrast](contrast.md)(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Adjusts the contrast of the image. Lower values decrease the contrast of the image (the tonal range is narrowed), and higher values increase the contrast of the image (the tonal range is widened). A value of 1.0 has no effect. The contrast is defined as a value in the range `[0.0...2.0]`. Values outside of that range will be clipped to that range. Contrast adjustment is performed in log space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| contrast | Contrast expansion, between 0.0 and 2.0. 1.0 leaves contrast unaffected |
