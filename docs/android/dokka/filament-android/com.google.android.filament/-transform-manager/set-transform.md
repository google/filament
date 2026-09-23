//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[setTransform](set-transform.md)

# setTransform

[main]\
open fun [setTransform](set-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Sets a local transform of a transform component.

#### Parameters

main

| | |
|---|---|
| ci | The instance of the transform component to set the local transform to. |
| localTransform | The local transform (i.e. relative to the parent). |

#### See also

| |
|---|
| [getTransform](get-transform.md) |

[main]\
open fun [setTransform](set-transform.md)(ci: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Sets a local transform of a transform component and keeps double precision translation. 

All other values of the transform are stored at single precision.

#### Parameters

main

| | |
|---|---|
| ci | The instance of the transform component to set the local transform to. |
| localTransform | The local transform (i.e. relative to the parent). |

#### See also

| |
|---|
| [getTransform](get-transform.md) |
