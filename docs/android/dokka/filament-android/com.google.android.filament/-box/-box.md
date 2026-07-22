//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)/[Box](-box.md)

# Box

[main]\
constructor()

Default-initializes the 3D box to have a center and half-extent of (0,0,0).

[main]\
constructor(centerX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Initializes the 3D box from its center and half-extent.

[main]\
constructor(center: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, halfExtent: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Initializes the 3D box from its center and half-extent.

#### Parameters

main

| | |
|---|---|
| center | a float array with XYZ coordinates representing the center of the box |
| halfExtent | a float array with XYZ coordinates representing half the size of the box in each dimension |
