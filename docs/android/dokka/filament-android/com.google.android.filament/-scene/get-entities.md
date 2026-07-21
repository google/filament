//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[getEntities](get-entities.md)

# getEntities

[main]\
open fun [getEntities](get-entities.md)(outArray: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;

Returns the list of all entities in the Scene. If outArray is provided and large enough, it is used to store the list and returned, otherwise a new array is allocated and returned.

#### Return

outArray if it was used or a newly allocated array.

#### Parameters

main

| | |
|---|---|
| outArray | an array to store the list of entities in the scene. |

#### See also

| |
|---|
| [getEntityCount](get-entity-count.md) |

[main]\
open fun [getEntities](get-entities.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;

Returns the list of all entities in the Scene in a newly allocated array.

#### Return

an array containing the list of all entities in the scene.

#### See also

| |
|---|
| [getEntityCount](get-entity-count.md) |
