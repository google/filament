//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getModelMatrix](get-model-matrix.md)

# getModelMatrix

[main]\
open fun [getModelMatrix](get-model-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Retrieves the camera's model matrix. The model matrix encodes the camera position and orientation, or pose.

#### Return

A 16-float array containing the camera's pose as a column-major matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-float array where the model matrix will be stored, or null in which case a new array is allocated. |

[main]\
open fun [getModelMatrix](get-model-matrix.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

Retrieves the camera's model matrix. The model matrix encodes the camera position and orientation, or pose.

#### Return

A 16-double array containing the camera's pose as a column-major matrix.

#### Parameters

main

| | |
|---|---|
| out | A 16-double array where the model matrix will be stored, or null in which case a new array is allocated. |
