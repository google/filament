//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[destroyEntity](destroy-entity.md)

# destroyEntity

[main]\
open fun [destroyEntity](destroy-entity.md)(entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Destroys all Filament-known components from this `entity`. 

 This method destroys Filament components only, not the `entity` itself. To destroy the `entity` use `EntityManager#destroy`. It is recommended to destroy components individually before destroying their `entity`, this gives more control as to when the destruction really happens. Otherwise, orphaned components are garbage collected, which can happen at a later time. Even when component are garbage collected, the destruction of their `entity` terminates their participation immediately.

#### Parameters

main

| | |
|---|---|
| entity | the `entity` to destroy |
