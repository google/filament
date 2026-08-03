//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setPosition](set-position.md)

# setPosition

[main]\
open fun [setPosition](set-position.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), x: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), y: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), z: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's position. 

**note:** The Light's position is ignored for directional lights ([DIRECTIONAL](-type/-d-i-r-e-c-t-i-o-n-a-l/index.md) or [SUN](-type/-s-u-n/index.md)) 

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| x | Light's position x coordinate in world space. The default is 0. |
| y | Light's position y coordinate in world space. The default is 0. |
| z | Light's position z coordinate in world space. The default is 0. |

#### See also

| |
|---|
| [LightManager.Builder](-builder/position.md) |
