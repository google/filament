//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)/[intensity](intensity.md)

# intensity

[main]\
open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)

Skybox intensity when no IndirectLight is set on the Scene. 

This call is ignored when an IndirectLight is set on the Scene, and the intensity of the IndirectLight is used instead.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| envIntensity | Scale factor applied to the skybox texel values such that the result is in lux, or lumen/m^2 (default = 30000) |

#### See also

| |
|---|
| [IndirectLight.Builder](../../-indirect-light/-builder/intensity.md) |
