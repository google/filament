//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ToneMapper](../index.md)/[Generic](index.md)/[Generic](-generic.md)

# Generic

[main]\
constructor()

Builds a new generic tone mapper parameterized to closely approximate the [ACESLegacy](../-a-c-e-s-legacy/index.md) tone mapper. The default values are: 

- contrast = 1.55f
- midGrayIn = 0.18f
- midGrayOut = 0.215f
- hdrMax = 10.0f

[main]\
constructor(contrast: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayIn: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), midGrayOut: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), hdrMax: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Builds a new generic tone mapper.

#### Parameters

main

| | |
|---|---|
| contrast | : controls the contrast of the curve, must be >0.0, values in the range 0.5..2.0 are recommended. |
| midGrayIn | : sets the input middle gray, between 0.0 and 1.0. |
| midGrayOut | : sets the output middle gray, between 0.0 and 1.0. |
| hdrMax | : defines the maximum input value that will be mapped to output white. Must be >= 1.0. |
