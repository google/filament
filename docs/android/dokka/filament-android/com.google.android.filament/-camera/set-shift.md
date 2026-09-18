//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[setShift](set-shift.md)

# setShift

[main]\
open fun [setShift](set-shift.md)(shiftx: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html), shifty: [Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html))

Sets an additional matrix that shifts the projection matrix. 

By default, this is an identity matrix.

#### Parameters

main

| | |
|---|---|
| shiftx | (x component) x and y translation added to the projection matrix, specified in NDC coordinates, that is, if the translation must be specified in pixels, shift must be scaled by 1.0 / { viewport.width, viewport.height }. |
| shifty | (y component) x and y translation added to the projection matrix, specified in NDC coordinates, that is, if the translation must be specified in pixels, shift must be scaled by 1.0 / { viewport.width, viewport.height }. |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |

[main]\
open fun [setShift](set-shift.md)(shift: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Sets an additional matrix that shifts the projection matrix. 

By default, this is an identity matrix.

#### Parameters

main

| | |
|---|---|
| shift | x and y translation added to the projection matrix, specified in NDC coordinates, that is, if the translation must be specified in pixels, shift must be scaled by 1.0 / { viewport.width, viewport.height }. |

#### See also

| |
|---|
| setProjection |
| [setLensProjection](set-lens-projection.md) |
| setCustomProjection |
