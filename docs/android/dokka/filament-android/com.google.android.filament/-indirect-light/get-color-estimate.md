//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndirectLight](index.md)/[getColorEstimate](get-color-estimate.md)

# getColorEstimate

[main]\
open fun [getColorEstimate](get-color-estimate.md)(colorIntensity: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the color and relative intensity of the environment in a given direction. 

This can be used to set the color and intensity of a directional light. In this case make sure to multiply this relative intensity by the the intensity of this indirect light.

#### Return

A vector of 4 floats where the first 3 components represent the linear color and the 4th component represents the intensity of the dominant light

#### Parameters

main

| | |
|---|---|
| colorIntensity | an array of 4 floats to receive the result or `null` |
| sh | pre-scaled 3-bands spherical harmonics |
| x | the x coordinate of a unit vector representing the direction of the light |
| y | the x coordinate of a unit vector representing the direction of the light |
| z | the x coordinate of a unit vector representing the direction of the light |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
| getDirectionEstimate |
| [getIntensity](get-intensity.md) |
| [setIntensity](set-intensity.md) |

[main]\
open fun [~~getColorEstimate~~](get-color-estimate.md)(colorIntensity: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

---

### Deprecated

---

#### Deprecated
