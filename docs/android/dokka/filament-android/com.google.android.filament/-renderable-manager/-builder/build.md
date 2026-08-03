//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Adds the Renderable component to an entity. 

If this component already exists on the given entity and the construction is successful, it is first destroyed as if [destroy](../destroy.md) was called.

#### Parameters

main

| | |
|---|---|
| engine | reference to the `Engine` to associate this renderable with |
| entity | entity to add the renderable component to |

#### Throws

| | |
|---|---|
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources, or if a parameter to a builder function was invalid. |
