//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setBlendOrderAt](set-blend-order-at.md)

# setBlendOrderAt

[main]\
open fun [setBlendOrderAt](set-blend-order-at.md)(instance: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), primitiveIndex: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), blendOrder: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Changes the drawing order for blended primitives. The drawing order is either global or local (default) to this Renderable. In either case, the Renderable priority takes precedence.

#### Parameters

main

| | |
|---|---|
| instance | the renderable of interest |
| primitiveIndex | the primitive of interest |
| blendOrder | draw order number (0 by default). Only the lowest 15 bits are used. |

#### See also

| |
|---|
| [RenderableManager.Builder](-builder/blend-order.md) |
