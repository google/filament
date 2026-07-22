//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[View](../index.md)/[FogOptions](index.md)/[heightFalloff](height-falloff.md)

# heightFalloff

[main]\
open var [heightFalloff](height-falloff.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)

How fast the fog dissipates with the altitude. heightFalloff has a unit of [1/m]. It can be expressed as 1/H, where H is the altitude change in world units [m] that causes a factor 2.78 (e) change in fog density. 

A falloff of 0 means the fog density is constant everywhere and may result is slightly faster computations.

In `linearFog` mode, only use to compute the slope of the linear equation. Completely ignored if set to 0.
