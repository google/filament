//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[priority](priority.md)

# priority

[main]\
open fun [priority](priority.md)(priority: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Provides coarse-grained control over draw order. 

In general Filament reserves the right to re-order renderables to allow for efficient rendering. However clients can control ordering at a coarse level using priority. The priority is applied separately for opaque and translucent objects, that is, opaque objects are always drawn before translucent objects regardless of the priority.

For example, this could be used to draw a semitransparent HUD, if a client wishes to avoid using a separate View for the HUD. Note that priority is completely orthogonal to [layerMask](layer-mask.md), which merely controls visibility.

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
| [RenderableManager.Builder](blend-order.md) |
| [setPriority](../set-priority.md) |
| [setBlendOrderAt](../set-blend-order-at.md) |
