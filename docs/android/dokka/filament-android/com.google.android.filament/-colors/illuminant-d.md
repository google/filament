//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[illuminantD](illuminant-d.md)

# illuminantD

[main]\
open fun [illuminantD](illuminant-d.md)(temperature: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts a CIE standard illuminant series D to a linear RGB color in sRGB space. The temperature must be expressed in Kelvin and must be in the range 4,000K to 25,000K.

#### Return

an RGB float array of size 3 with the result of the conversion

#### Parameters

main

| | |
|---|---|
| temperature | the temperature, in Kelvin |
