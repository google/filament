//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[spotLightCone](spot-light-cone.md)

# spotLightCone

[main]\
open fun [spotLightCone](spot-light-cone.md)(inner: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), outer: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Defines a spot light'st angular falloff attenuation. 

A spot light is defined by a position, a direction and two cones, `inner` and `outer`. These two cones are used to define the angular falloff attenuation of the spot light and are defined by the angle from the center axis to where the falloff begins (i.e. cones are defined by their half-angle).

Both inner and outer are silently clamped to a minimum value of 0.5 degrees (~0.00873 radians) to avoid floating-point precision issues during rendering.

The spot light cone is ignored for directional and point lights.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| inner | inner cone angle in *radians* between 0.00873 and `outer` |
| outer | outer cone angle in *radians* between 0.00873 inner and π/2 |

#### See also

| |
|---|
| Type.SPOT |
| Type.FOCUSED_SPOT |
