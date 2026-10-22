//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndirectLight](index.md)/[getColorEstimate](get-color-estimate.md)

# getColorEstimate

[main]\
open fun [getColorEstimate](get-color-estimate.md)(sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the color and relative intensity of the environment represented by spherical harmonics in a given direction. 

This can be used to set the color and intensity of a directional light. In this case make sure to multiply this relative intensity by the the intensity of this indirect light.

#### Return

A vector of 4 floats where the first 3 components represent the linear color and the 4th component represents the intensity of the dominant light

#### Parameters

main

| | |
|---|---|
| sh | 3-band spherical harmonics |
| direction | a unit vector representing the direction of the light to estimate the color of. Typically this the value returned by getDirectionEstimate(). |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
| getDirectionEstimate |
| [getIntensity](get-intensity.md) |
| [setIntensity](set-intensity.md) |

[main]\
open fun [getColorEstimate](get-color-estimate.md)(sh: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, directionx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directiony: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directionz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the color and relative intensity of the environment represented by spherical harmonics in a given direction. 

This can be used to set the color and intensity of a directional light. In this case make sure to multiply this relative intensity by the the intensity of this indirect light.

#### Return

A vector of 4 floats where the first 3 components represent the linear color and the 4th component represents the intensity of the dominant light

#### Parameters

main

| | |
|---|---|
| sh | 3-band spherical harmonics |
| directionx | (x component) a unit vector representing the direction of the light to estimate the color of. Typically this the value returned by getDirectionEstimate(). |
| directiony | (y component) a unit vector representing the direction of the light to estimate the color of. Typically this the value returned by getDirectionEstimate(). |
| directionz | (z component) a unit vector representing the direction of the light to estimate the color of. Typically this the value returned by getDirectionEstimate(). |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
| getDirectionEstimate |
| [getIntensity](get-intensity.md) |
| [setIntensity](set-intensity.md) |

[main]\
open fun [getColorEstimate](get-color-estimate.md)(direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the color and relative intensity of the environment represented by spherical harmonics in a given direction. 

Spherical harmonics must be set in the Builder or the result is undefined.

#### See also

| |
|---|
| [getColorEstimate(float[], float[])](get-color-estimate.md) |
| [IndirectLight.Builder](-builder/radiance.md) |

[main]\
open fun [getColorEstimate](get-color-estimate.md)(directionx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directiony: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directionz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Helper to estimate the color and relative intensity of the environment represented by spherical harmonics in a given direction. 

Spherical harmonics must be set in the Builder or the result is undefined.

#### Parameters

main

| | |
|---|---|
| directionx | (x component) |
| directiony | (y component) |
| directionz | (z component) |

#### See also

| |
|---|
| [getColorEstimate(float[], float[])](get-color-estimate.md) |
| [IndirectLight.Builder](-builder/radiance.md) |
