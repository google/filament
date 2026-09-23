//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getWorldTransform](get-world-transform.md)

# getWorldTransform

[main]\
open fun [getWorldTransform](get-world-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), out: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;

Return the world transform of a transform component.

#### Return

The world transform of the component (i.e. relative to the root). This is the composition of this component's local transform with its parent's world transform.

#### Parameters

main

| | |
|---|---|
| ci | The instance of the transform component to query the world transform from. |

#### See also

| |
|---|
| setTransform |
