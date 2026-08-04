//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getLeftVector](get-left-vector.md)

# getLeftVector

[main]\
open fun [getLeftVector](get-left-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Retrieves the camera left unit vector in world space, that is a unit vector that points to the left of the camera.

#### Return

A 3-float array containing the camera's left vector in world units.

#### Parameters

main

| | |
|---|---|
| out | A 3-float array where the left vector will be stored, or null in which case a new array is allocated. |
