//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[IndirectLight](index.md)/[getRotation](get-rotation.md)

# getRotation

[main]\
open fun [getRotation](get-rotation.md)(rotation: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Returns the rigid-body transformation applied to the IBL.

#### Return

the `rotation` paramter if it was provided, or a newly allocated float array containing the rigid-body transformation applied to the IBL

#### Parameters

main

| | |
|---|---|
| rotation | an array of 9 floats to receive the rigid-body transformation applied to the IBL or `null` |
