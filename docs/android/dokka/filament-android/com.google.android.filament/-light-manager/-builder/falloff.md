//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[falloff](falloff.md)

# falloff

[main]\
open fun [falloff](falloff.md)(radius: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Set the falloff distance for point lights and spot lights. 

 At the falloff distance, the light has no more effect on objects. 

 The falloff distance essentially defines a **sphere of influence** around the light, and therefore has an impact on performance. Larger falloffs might reduce performance significantly, especially when many lights are used. 

 Try to avoid having a large number of light's spheres of influence overlap. 

 The Light's falloff is ignored for directional lights ([DIRECTIONAL](../-type/-d-i-r-e-c-t-i-o-n-a-l/index.md) or [SUN](../-type/-s-u-n/index.md))

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| radius | Falloff distance in world units. Default is 1 meter. |
