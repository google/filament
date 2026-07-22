//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Box](index.md)

# Box

[main]\
open class [Box](index.md)

An axis-aligned 3D box represented by its center and half-extent. The half-extent is a vector representing the distance from the center to the edge of the box in each dimension. For example, a box of size 2 units in X, 4 units in Y, and 10 units in Z would have a half-extent of (1, 2, 5).

## Constructors

| | |
|---|---|
| [Box](-box.md) | [main]<br>constructor()<br>Default-initializes the 3D box to have a center and half-extent of (0,0,0).<br>constructor(centerX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Initializes the 3D box from its center and half-extent.<br>constructor(center: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;, halfExtent: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Initializes the 3D box from its center and half-extent. |

## Functions

| Name | Summary |
|---|---|
| [getCenter](get-center.md) | [main]<br>open fun [getCenter](get-center.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the center of the 3D box. |
| [getHalfExtent](get-half-extent.md) | [main]<br>open fun [getHalfExtent](get-half-extent.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the half-extent from the center of the 3D box. |
| [setCenter](set-center.md) | [main]<br>open fun [setCenter](set-center.md)(centerX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), centerZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the center of of the 3D box. |
| [setHalfExtent](set-half-extent.md) | [main]<br>open fun [setHalfExtent](set-half-extent.md)(halfExtentX: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentY: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), halfExtentZ: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))<br>Sets the half-extent of the 3D box. |
