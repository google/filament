//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[setParent](set-parent.md)

# setParent

[main]\
open fun [setParent](set-parent.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), newParent: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Re-parents an entity to a new one.

#### Parameters

main

| | |
|---|---|
| i | the [EntityInstance](../-entity-instance/index.md) of the transform component to re-parent |
| newParent | the [EntityInstance](../-entity-instance/index.md) of the new parent transform. It is an error to re-parent an entity to a descendant and will cause undefined behaviour. |

#### See also

| |
|---|
| [getInstance](get-instance.md) |
