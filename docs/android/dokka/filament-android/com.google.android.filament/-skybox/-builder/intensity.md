//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)/[intensity](intensity.md)

# intensity

[main]\
open fun [intensity](intensity.md)(envIntensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Skybox.Builder](index.md)

Sets the `Skybox` intensity when no [IndirectLight](../../-indirect-light/index.md) is set on the [Scene](../../-scene/index.md). 

This call is ignored when an [IndirectLight](../../-indirect-light/index.md) is set on the [Scene](../../-scene/index.md), and the intensity of the [IndirectLight](../../-indirect-light/index.md) is used instead.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| envIntensity | Scale factor applied to the skybox texel values such that the result is in *lux*, or *lumen/m^2* (default = 30000) |

#### See also

| |
|---|
| [IndirectLight.Builder](../../-indirect-light/-builder/intensity.md) |
