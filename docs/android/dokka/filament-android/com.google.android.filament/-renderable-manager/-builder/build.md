//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Adds the Renderable component to an entity.

#### Return

Success if the component was created successfully, Error otherwise.

If exceptions are disabled and an error occurs, this function is a no-op. Success can be checked by looking at the return value.

If this component already exists on the given entity and the construction is successful, it is first destroyed as if destroy(utils::Entity e) was called. In case of error, the existing component is unmodified.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the filament::Engine to associate this Renderable with. |
| entity | Entity to add the Renderable component to. |
