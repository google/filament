//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getViewMatrix](get-view-matrix.md)

# getViewMatrix

[main]\
open fun [getViewMatrix](get-view-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Retrieves the camera's view matrix. The view matrix is the inverse of the model matrix.

#### Return

A 16-float array containing the camera's column-major view matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-float array where the view matrix will be stored, or null in which case a new array is allocated. |

[main]\
open fun [getViewMatrix](get-view-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Retrieves the camera's view matrix. The view matrix is the inverse of the model matrix.

#### Return

A 16-double array containing the camera's column-major view matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-double array where the model view will be stored, or null in which case a new array is allocated. |
