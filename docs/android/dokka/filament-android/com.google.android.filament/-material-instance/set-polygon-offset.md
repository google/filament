//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[MaterialInstance](index.md)/[setPolygonOffset](set-polygon-offset.md)

# setPolygonOffset

[main]\
open fun [setPolygonOffset](set-polygon-offset.md)(scale: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), constant: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html))

Sets a polygon offset that will be applied to all renderables drawn with this material instance. The value of the offset is scale * dz + r * constant, where dz is the change in depth relative to the screen area of the triangle, and r is the smallest value that is guaranteed to produce a resolvable offset for a given implementation. This offset is added before the depth test. Warning: using a polygon offset other than zero has a significant negative performance impact, as most implementations have to disable early depth culling. DO NOT USE unless absolutely necessary.

#### Parameters

main

| | |
|---|---|
| scale | scale factor used to create a variable depth offset for each triangle |
| constant | scale factor used to create a constant depth offset for each triangle |
