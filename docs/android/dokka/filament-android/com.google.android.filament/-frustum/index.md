//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Frustum](index.md)

# Frustum

[main]\
open class [Frustum](index.md)

A frustum defined by six planes

## Constructors

| | |
|---|---|
| [Frustum](-frustum.md) | [main]<br>constructor()constructor(pv: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Creates a frustum from a projection matrix in GL convention (usually the projection * view matrix) |

## Types

| Name | Summary |
|---|---|
| [Plane](-plane/index.md) | [main]<br>enum [Plane](-plane/index.md) |

## Functions

| Name | Summary |
|---|---|
| [contains](contains.md) | [main]<br>open fun [contains](contains.md)(p: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>open fun [contains](contains.md)(px: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), py: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), pz: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Returns whether the frustum contains a given point. |
| [getNormalizedPlane](get-normalized-plane.md) | [main]<br>open fun [getNormalizedPlane](get-normalized-plane.md)(plane: [Frustum.Plane](-plane/index.md), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns the plane equation parameters with normalized normals |
| [getNormalizedPlanes](get-normalized-planes.md) | [main]<br>open fun [getNormalizedPlanes](get-normalized-planes.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>[main]<br>open fun [getNormalizedPlanes](get-normalized-planes.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;<br>Returns a copy of all six frustum planes in left, right, bottom, top, far, near order |
| [intersects](intersects.md) | [main]<br>open fun [intersects](intersects.md)(box: [Box](../-box/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a box intersects the frustum (i.e.<br>[main]<br>open fun [intersects](intersects.md)(sphere: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>open fun [intersects](intersects.md)(spherex: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherey: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherez: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), spherew: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Returns whether a sphere intersects the frustum (i.e. |
| [setProjection](set-projection.md) | [main]<br>open fun [setProjection](set-projection.md)(pv: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)<br>Sets the frustum from the given projection matrix |
