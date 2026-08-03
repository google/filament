//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[spotLightCone](spot-light-cone.md)

# spotLightCone

[main]\
open fun [spotLightCone](spot-light-cone.md)(inner: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), outer: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Defines a spot light's angular falloff attenuation. 

 A spot light is defined by a position, a direction and two cones, inner and outer. These two cones are used to define the angular falloff attenuation of the spot light and are defined by the angle from the center axis to where the falloff begins (i.e. cones are defined by their half-angle). 

**note:** The spot light cone is ignored for directional and point lights. 

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| inner | inner cone angle in *radian* between 0 and pi/2 |
| outer | outer cone angle in *radian* between inner and pi/2 |

#### See also

| |
|---|
| [LightManager.Type](../-type/-f-o-c-u-s-e-d_-s-p-o-t/index.md) |
