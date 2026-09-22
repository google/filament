//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[intensityCandela](intensity-candela.md)

# intensityCandela

[main]\
open fun [intensityCandela](intensity-candela.md)(intensity: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial intensity of a spot or point light in candela. 

This method is equivalent to calling intensity(float intensity) for directional lights (Type.DIRECTIONAL or Type.SUN).

#### Return

This Builder, for chaining calls.

This method overrides any prior calls to intensity or intensityCandela.

#### Parameters

main

| | |
|---|---|
| intensity | Luminous intensity in *candela*. |
