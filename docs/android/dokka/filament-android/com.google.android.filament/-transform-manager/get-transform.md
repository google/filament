//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getTransform](get-transform.md)

# getTransform

[main]\
open fun [getTransform](get-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Returns the local transform of a transform component.

#### Return

The local transform of the component (i.e. relative to the parent). This always returns the value set by setTransform().

#### Parameters

main

| | |
|---|---|
| ci | The instance of the transform component to query the local transform from. |

#### See also

| |
|---|
| setTransform |
