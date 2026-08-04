//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setSpotLightCone](set-spot-light-cone.md)

# setSpotLightCone

[main]\
open fun [setSpotLightCone](set-spot-light-cone.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), inner: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), outer: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates a spot light's cone as angles

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| inner | inner cone angle in *radians* between 0 and pi/2 |
| outer | outer cone angle in *radians* between inner and pi/2 |

#### See also

| |
|---|
| [LightManager.Builder](-builder/spot-light-cone.md) |
