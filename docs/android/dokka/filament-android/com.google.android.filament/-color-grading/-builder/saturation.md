//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[saturation](saturation.md)

# saturation

[main]\
open fun [saturation](saturation.md)(saturation: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Adjusts the saturation of the image. Lower values decrease intensity of the colors present in the image, and higher values increase the intensity of the colors in the image. A value of 1.0 has no effect. The saturation is defined as a value in the range `[0.0...2.0]`. Values outside of that range will be clipped to that range. Saturation adjustment is performed in linear space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| saturation | Saturation, between 0.0 and 2.0. 1.0 leaves saturation unaffected |
