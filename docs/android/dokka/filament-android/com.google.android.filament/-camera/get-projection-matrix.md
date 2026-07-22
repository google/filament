//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getProjectionMatrix](get-projection-matrix.md)

# getProjectionMatrix

[main]\
open fun [getProjectionMatrix](get-projection-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Retrieves the camera's projection matrix. The projection matrix used for rendering always has its far plane set to infinity. This is why it may differ from the matrix set through setProjection() or setLensProjection().

#### Return

A 16-float array containing the camera's projection as a column-major matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-float array where the projection matrix will be stored, or null in which case a new array is allocated. |
