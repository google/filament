//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Skybox](../index.md)/[Builder](index.md)/[priority](priority.md)

# priority

[main]\
open fun [priority](priority.md)(priority: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Skybox.Builder](index.md)

Set the rendering priority of the Skybox. By default, it is set to the lowest priority (7) such that the Skybox is always rendered after the opaque objects, to reduce overdraw when depth culling is enabled.

#### Return

Builder reference for chaining calls.

#### Parameters

main

| | |
|---|---|
| priority | clamped to the range [0..7], defaults to 4; 7 is lowest priority (rendered last). |

#### See also

| |
|---|
| [RenderableManager.Builder](../../-renderable-manager/-builder/priority.md) |
