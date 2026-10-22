//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[hasEntity](has-entity.md)

# hasEntity

[main]\
open fun [hasEntity](has-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Returns true if the given entity is in the Scene. 

Entities destroyed with destroy are removed lazily the next time a [View](../-view/index.md) attached to this Scene is rendered or detached; until then they may still be reported as present.

#### Return

Whether the given entity is in the Scene.
