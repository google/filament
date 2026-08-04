//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[whiteBalance](white-balance.md)

# whiteBalance

[main]\
open fun [whiteBalance](white-balance.md)(temperature: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), tint: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [ColorGrading.Builder](index.md)

Adjusts the while balance of the image. This can be used to remove color casts and correct the appearance of the white point in the scene, or to alter the overall chromaticity of the image for artistic reasons (to make the image appear cooler or warmer for instance). The while balance adjustment is defined with two values: 

- Temperature, to modify the color temperature. This value will modify the colors on a blue/yellow axis. Lower values apply a cool color temperature, and higher values apply a warm color temperature. The lowest value, -1.0f, is equivalent to a temperature of 50,000K. The highest value, 1.0f, is equivalent to a temperature of 2,000K.
- Tint, to modify the colors on a green/magenta axis. The lowest value, -1.0f, will apply a strong green cast, and the highest value, 1.0f, will apply a strong magenta cast.

 Both values are expected to be in the range `[-1.0..+1.0]`. Values outside of that range will be clipped to that range.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| temperature | Modification on the blue/yellow axis, as a value between -1.0 and +1.0. |
| tint | Modification on the green/magenta axis, as a value between -1.0 and +1.0. |
