//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Colors](index.md)/[cct](cct.md)

# cct

[main]\
open fun [cct](cct.md)(temperature: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Converts a correlated color temperature to a linear RGB color in sRGB space. The temperature must be expressed in Kelvin and must be in the range 1,000K to 15,000K.

#### Return

an RGB float array of size 3 with the result of the conversion

#### Parameters

main

| | |
|---|---|
| temperature | the temperature, in Kelvin |
