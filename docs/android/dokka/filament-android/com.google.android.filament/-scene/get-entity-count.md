//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[getEntityCount](get-entity-count.md)

# getEntityCount

[main]\
open fun [getEntityCount](get-entity-count.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)

Returns the number of Entities in the Scene. 

Entities destroyed with destroy are removed lazily the next time a [View](../-view/index.md) attached to this Scene is rendered or detached; until then they may still be counted.

#### Return

Total number of Entities in the Scene.
