//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getCullingProjectionMatrix](get-culling-projection-matrix.md)

# getCullingProjectionMatrix

[main]\
open fun [getCullingProjectionMatrix](get-culling-projection-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Retrieves the camera's culling matrix. The culling matrix is the same as the projection matrix, except the far plane is finite.

#### Return

A 16-float array containing the camera's projection as a column-major matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-float array where the projection matrix will be stored, or null in which case a new array is allocated. |
