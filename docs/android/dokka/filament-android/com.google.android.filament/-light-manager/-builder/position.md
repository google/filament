//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[position](position.md)

# position

[main]\
open fun [position](position.md)(positionx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), positiony: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), positionz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [LightManager.Builder](index.md)

Sets the initial position of the light in world space. 

The Light's position is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)

#### Return

This Builder, for chaining calls.

[main]\
open fun [position](position.md)(position: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [LightManager.Builder](index.md)

Sets the initial position of the light in world space. 

The Light's position is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| position | Light's position in world space. The default is at the origin. |
