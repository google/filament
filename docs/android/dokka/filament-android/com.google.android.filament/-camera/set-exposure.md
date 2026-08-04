//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setExposure](set-exposure.md)

# setExposure

[main]\
open fun [setExposure](set-exposure.md)(aperture: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), shutterSpeed: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), sensitivity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Sets this camera's exposure (default is f/16, 1/125s, 100 ISO) The exposure ultimately controls the scene's brightness, just like with a real camera. The default values provide adequate exposure for a camera placed outdoors on a sunny day with the sun at the zenith. With the default parameters, the scene must contain at least one Light of intensity similar to the sun (e.g.: a 100,000 lux directional light) and/or an indirect light of appropriate intensity (30,000).

#### Parameters

main

| | |
|---|---|
| aperture | Aperture in f-stops, clamped between 0.5 and 64. A lower aperture value increases the exposure, leading to a brighter scene. Realistic values are between 0.95 and 32. |
| shutterSpeed | Shutter speed in seconds, clamped between 1/25,000 and 60. A lower shutter speed increases the exposure. Realistic values are between 1/8000 and 30. |
| sensitivity | Sensitivity in ISO, clamped between 10 and 204,800. A higher sensitivity increases the exposure. Realistic values are between 50 and 25600. |

#### See also

| |
|---|
| [LightManager](../-light-manager/index.md) |
| [setExposure(float)](set-exposure.md) |

[main]\
open fun [setExposure](set-exposure.md)(exposure: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Sets this camera's exposure directly. Calling this method will set the aperture to 1.0, the shutter speed to 1.2 and the sensitivity will be computed to match the requested exposure (for a desired exposure of 1.0, the sensitivity will be set to 100 ISO). This method is useful when trying to match the lighting of other engines or tools. Many engines/tools use unit-less light intensities, which can be matched by setting the exposure manually. This can be typically achieved by setting the exposure to 1.0.

#### See also

| |
|---|
| [LightManager](../-light-manager/index.md) |
| [setExposure(float, float, float)](set-exposure.md) |
