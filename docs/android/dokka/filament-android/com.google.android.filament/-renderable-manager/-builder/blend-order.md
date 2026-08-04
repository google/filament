//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[blendOrder](blend-order.md)

# blendOrder

[main]\
open fun [blendOrder](blend-order.md)(index: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), blendOrder: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Sets the drawing order for blended primitives. The drawing order is either global or local (default) to this Renderable. In either case, the Renderable priority takes precedence.

#### Parameters

main

| | |
|---|---|
| index | the primitive of interest |
| blendOrder | draw order number (0 by default). Only the lowest 15 bits are used. |
