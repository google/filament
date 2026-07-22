//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getUpVector](get-up-vector.md)

# getUpVector

[main]\
open fun [getUpVector](get-up-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Retrieves the camera up unit vector in world space, that is a unit vector that points up with respect to the camera.

#### Return

A 3-float array containing the camera's up vector in world units.

#### Parameters

main

| | |
|---|---|
| out | A 3-float array where the up vector will be stored, or null in which case a new array is allocated. |
