//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[setTransform](set-transform.md)

# setTransform

[main]\
open fun [setTransform](set-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;)

Sets a local transform of a transform component. 

This operation can be slow if the hierarchy of transform is too deep, and this will be particularly bad when updating a lot of transforms. In that case, consider using [openLocalTransformTransaction](open-local-transform-transaction.md) / [commitLocalTransformTransaction](commit-local-transform-transaction.md).

#### Parameters

main

| | |
|---|---|
| i | the [EntityInstance](../-entity-instance/index.md) of the transform component to set the local transform to. |
| localTransform | the local transform (i.e. relative to the parent). |

#### See also

| |
|---|
| getTransform |

[main]\
open fun [setTransform](set-transform.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;)

Sets a local transform of a transform component. 

This operation can be slow if the hierarchy of transform is too deep, and this will be particularly bad when updating a lot of transforms. In that case, consider using [openLocalTransformTransaction](open-local-transform-transaction.md) / [commitLocalTransformTransaction](commit-local-transform-transaction.md).

#### Parameters

main

| | |
|---|---|
| i | the [EntityInstance](../-entity-instance/index.md) of the transform component to set the local transform to. |
| localTransform | the local transform (i.e. relative to the parent). |

#### See also

| |
|---|
| [getTransform(int, double[])](get-transform.md) |
| [getWorldTransform(int, double[])](get-world-transform.md) |
