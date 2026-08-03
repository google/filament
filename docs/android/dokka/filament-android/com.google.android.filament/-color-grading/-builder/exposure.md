//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[exposure](exposure.md)

# exposure

[main]\
open fun [exposure](exposure.md)(exposure: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Adjusts the exposure of this image. The exposure is specified in stops: each stop brightens (positive values) or darkens (negative values) the image by a factor of 2. This means that an exposure of 3 will brighten the image 8 times more than an exposure of 0 (2^3 = 8 and 2^0 = 1). Contrary to the camera's exposure, this setting is applied after all post-processing (bloom, etc.) are applied.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| exposure | Value in EV stops. Can be negative, 0, or positive. |
