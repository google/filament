//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[direction](direction.md)

# direction

[main]\
open fun [direction](direction.md)(directionx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directiony: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directionz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial direction of a light in world space. 

The Light's direction is ignored for Type.POINT lights.

#### Return

This Builder, for chaining calls.

[main]\
open fun [direction](direction.md)(direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [LightManager.Builder](index.md)

Sets the initial direction of a light in world space. 

The Light's direction is ignored for Type.POINT lights.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| direction | Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
