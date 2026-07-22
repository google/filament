//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setDirection](set-direction.md)

# setDirection

[main]\
open fun [setDirection](set-direction.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's direction 

 The light direction is specified in world space and should be a unit vector. 

**note:** The Light's direction is ignored for [POINT](-type/-p-o-i-n-t/index.md) lights. 

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| x | light's direction x coordinate (default is 0) |
| y | light's direction y coordinate (default is -1) |
| z | light's direction z coordinate (default is 0) |

#### See also

| |
|---|
| [LightManager.Builder](-builder/direction.md) |
