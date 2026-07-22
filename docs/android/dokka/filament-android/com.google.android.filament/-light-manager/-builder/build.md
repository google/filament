//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[LightManager](../index.md)/[Builder](index.md)/[build](build.md)

# build

[main]\
open fun [build](build.md)(engine: [Engine](../../-engine/index.md), entity: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Adds the Light component to an entity. 

 If this component already exists on the given entity, it is first destroyed as if [destroy](../destroy.md) was called. 

**warning:** Currently, only 2048 lights can be created on a given Engine.

#### Parameters

main

| | |
|---|---|
| engine | Reference to the [Engine](../../-engine/index.md) to associate this light with. |
| entity | Entity to add the light component to. |

#### Throws

| | |
|---|---|
| [RuntimeException](https://developer.android.com/reference/kotlin/java/lang/RuntimeException.html) | if a runtime error occurred, such as running out of memory or other resources, or if a parameter to a builder function was invalid. |
