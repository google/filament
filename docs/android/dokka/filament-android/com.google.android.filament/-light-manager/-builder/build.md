//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Adds the Light component to an entity. 

Currently, only 2048 lights can be created on a given Engine.

#### Return

Success if the component was created successfully, Error otherwise.

If exceptions are disabled and an error occurs, this function is a no-op. Success can be checked by looking at the return value.

If this component already exists on the given entity, it is first destroyed as if destroy(utils::Entity e) was called.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this light with. |
| entity | Entity to add the light component to. |
