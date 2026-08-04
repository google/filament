//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[vibrance](vibrance.md)

# vibrance

[main]\
open fun [vibrance](vibrance.md)(vibrance: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Adjusts the saturation of the image based on the input color's saturation level. Colors with a high level of saturation are less affected than colors with low saturation levels. Lower vibrance values decrease intensity of the colors present in the image, and higher values increase the intensity of the colors in the image. A value of 1.0 has no effect. The vibrance is defined as a value in the range `[0.0...2.0]`. Values outside of that range will be clipped to that range. Vibrance adjustment is performed in linear space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| vibrance | Vibrance, between 0.0 and 2.0. 1.0 leaves vibrance unaffected |
