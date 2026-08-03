//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[TransformManager](index.md)/[getChildren](get-children.md)

# getChildren

[main]\
open fun [getChildren](get-children.md)(i: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), outEntities: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;

Gets a list of children for a transform component.

#### Return

Array of retrieved children [Entity](../-entity/index.md).

#### Parameters

main

| | |
|---|---|
| i | the [EntityInstance](../-entity-instance/index.md) of the transform component to get the children from. |
| outEntities | array to receive the result sized to the maximum number of children to retrieve. If `null` is given, a new suitable array sized to [getChildCount](get-child-count.md) is allocated. |
