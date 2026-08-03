//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md)

# setAutomaticInstancingEnabled

[main]\
open fun [setAutomaticInstancingEnabled](set-automatic-instancing-enabled.md)(enable: [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html))

Enables or disables automatic instancing of render primitives. Instancing of render primitive can greatly reduce CPU overhead but requires the instanced primitives to be identical (i.e. use the same geometry) and use the same MaterialInstance. If it is known that the scene doesn't contain any identical primitives, automatic instancing can have some overhead and it is then best to disable it. Disabled by default.

#### Parameters

main

| | |
|---|---|
| enable | true to enable, false to disable automatic instancing. |

#### See also

| |
|---|
| [RenderableManager](../-renderable-manager/index.md) |
| [MaterialInstance](../-material-instance/index.md) |
