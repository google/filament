//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[ColorGrading](../index.md)/[Builder](index.md)/[slopeOffsetPower](slope-offset-power.md)

# slopeOffsetPower

[main]\
open fun [slopeOffsetPower](slope-offset-power.md)(slope: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, offset: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, power: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [ColorGrading.Builder](index.md)

Applies a slope, offset, and power, as defined by the ASC CDL (American Society of Cinematographers Color Decision List) to the image. The CDL can be used to adjust the colors of different tonal ranges in the image. The ASC CDL is similar to the lift/gamma/gain controls found in many color grading tools. Lift is equivalent to a combination of offset and slope, gain is equivalent to slope, and gamma is equivalent to power. The slope and power values must be strictly positive. Values less than or equal to 0 will be clamped to a small positive value, offset can be any positive or negative value. Version 1.2 of the ASC CDL adds saturation control, which is here provided as a separate API. See the saturation() method for more information. Slope/offset/power adjustments are performed in log space.

#### Return

This Builder, for chaining calls

#### Parameters

main

| | |
|---|---|
| slope | Multiplier of the input color, must be a strictly positive number |
| offset | Added to the input color, can be a negative or positive number, including 0 |
| power | Power exponent of the input color, must be a strictly positive number |
