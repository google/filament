//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[priority](priority.md)

# priority

[main]\
open fun [priority](priority.md)(priority: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Provides coarse-grained control over draw order. 

In general Filament reserves the right to re-order renderables to allow for efficient rendering. However clients can control ordering at a coarse level using \em priority. The priority is applied separately for opaque and translucent objects, that is, opaque objects are always drawn before translucent objects regardless of the priority.

For example, this could be used to draw a semitransparent HUD on top of everything, without using a separate View. Note that priority is completely orthogonal to Builder::layerMask, which merely controls visibility.

The Skybox always using the lowest priority, so it's drawn last, which may improve performance.

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
| [RenderableManager.Builder](channel.md) |
| [RenderableManager](../set-blend-order-at.md) |
