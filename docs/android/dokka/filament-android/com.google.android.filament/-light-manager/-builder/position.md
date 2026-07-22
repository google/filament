//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[position](position.md)

# position

[main]\
open fun [position](position.md)(x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial position of the light in world space. 

**note:** The Light's position is ignored for directional lights ([DIRECTIONAL](../-type/-d-i-r-e-c-t-i-o-n-a-l/index.md) or [SUN](../-type/-s-u-n/index.md)) 

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| x | Light's position x coordinate in world space. The default is 0. |
| y | Light's position y coordinate in world space. The default is 0. |
| z | Light's position z coordinate in world space. The default is 0. |
