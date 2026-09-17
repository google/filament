//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getTransformAccurate](get-transform-accurate.md)

# getTransformAccurate

[main]\
open fun [getTransformAccurate](get-transform-accurate.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;

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
