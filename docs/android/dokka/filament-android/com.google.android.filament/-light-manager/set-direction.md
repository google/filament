//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[LightManager](index.md)/[setDirection](set-direction.md)

# setDirection

[main]\
open fun [setDirection](set-direction.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), directionx: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directiony: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), directionz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Dynamically updates the light's direction

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| directionx | (x component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
| directiony | (y component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |
| directionz | (z component) Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |

[main]\
open fun [setDirection](set-direction.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), direction: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Dynamically updates the light's direction

#### Parameters

main

| | |
|---|---|
| i | Instance of the component obtained from getInstance(). |
| direction | Light's direction in world space. Should be a unit vector. The default is {0,-1,0}. |

#### See also

| |
|---|
| com.google.android.filament.LightManager.Builder |
