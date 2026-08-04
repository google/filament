//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[create](create.md)

# create

[main]\
open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Creates a transform component and associates it with the given entity. The component is initialized with the identity transform. If this component already exists on the given entity, it is first destroyed as if [destroy](destroy.md) was called.

#### Parameters

main

| | |
|---|---|
| entity | an [Entity](../-entity/index.md) to associate a transform component to. |

#### See also

| |
|---|
| [destroy](destroy.md) |

[main]\
open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

open fun [create](create.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), parent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), localTransform: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Double](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-double/index.html)&gt;): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Creates a transform component with a parent and associates it with the given entity. If this component already exists on the given entity, it is first destroyed as if [destroy](destroy.md) was called.

#### Parameters

main

| | |
|---|---|
| entity | an [Entity](../-entity/index.md) to associate a transform component to. |
| parent | the [EntityInstance](../-entity-instance/index.md) of the parent transform |
| localTransform | the transform, relative to the parent, to initialize the transform component with. |

#### See also

| |
|---|
| [destroy](destroy.md) |
