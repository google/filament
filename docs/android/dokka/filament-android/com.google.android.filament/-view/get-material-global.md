//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[getMaterialGlobal](get-material-global.md)

# getMaterialGlobal

[main]\
open fun [getMaterialGlobal](get-material-global.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Get the value of the material global variables. All variable start with a default value of { 0, 0, 0, 1 }

#### Return

A 4-float array containing the current value of the variable.

#### Parameters

main

| | |
|---|---|
| index | index of the variable to set between 0 and 3. |
| out | A 4-float array where the value will be stored, or null in which case the array is allocated. |

#### See also

| |
|---|
| [setMaterialGlobal](set-material-global.md) |
