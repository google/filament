//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[direction](direction.md)

# direction

[main]\
open fun [direction](direction.md)(x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial direction of a light in world space. 

 The light direction is specified in world space and should be a unit vector. 

**note:** The Light's direction is ignored for [POINT](../-type/-p-o-i-n-t/index.md) lights. 

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| x | light's direction x coordinate (default is 0) |
| y | light's direction y coordinate (default is -1) |
| z | light's direction z coordinate (default is 0) |
