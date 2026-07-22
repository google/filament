//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Camera](index.md)/[getForwardVector](get-forward-vector.md)

# getForwardVector

[main]\
open fun [getForwardVector](get-forward-vector.md)(out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Retrieves the camera forward unit vector in world space, that is a unit vector that points in the direction the camera is looking at.

#### Return

A 3-float array containing the camera's forward vector in world units.

#### Parameters

main

| | |
|---|---|
| out | A 3-float array where the forward vector will be stored, or null in which case a new array is allocated. |
