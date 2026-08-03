//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Scene](index.md)/[addEntity](add-entity.md)

# addEntity

[main]\
open fun [addEntity](add-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Adds an [Entity](../-entity/index.md) to the `Scene`.

#### Parameters

main

| | |
|---|---|
| entity | the entity is ignored if it doesn't have a [RenderableManager](../-renderable-manager/index.md) component or [LightManager](../-light-manager/index.md) component.A given [Entity](../-entity/index.md) object can only be added once to a `Scene`. |
