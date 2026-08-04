//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[dimensions](dimensions.md)

# dimensions

[main]\
open fun [dimensions](dimensions.md)(dim: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [ColorGrading.Builder](index.md)

When color grading is implemented using a 3D LUT, this sets the dimension of the LUT. This overrides the value set by quality(). This setting has no effect if generating a 1D LUT. The default is 32

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| dim | The desired dimension of the LUT. Between 16 and 64. |
