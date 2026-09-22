//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderableManager](index.md)/[setBlendOrderAt](set-blend-order-at.md)

# setBlendOrderAt

[main]\
open fun [setBlendOrderAt](set-blend-order-at.md)(instance: Int, primitiveIndex: Int, order: Int)

Changes the drawing order for blended primitives. 

The drawing order is either global or local (default) to this Renderable. In either case, the Renderable priority takes precedence.

#### Parameters

main

| | |
|---|---|
| instance | the renderable of interest |
| primitiveIndex | the primitive of interest |
| order | draw order number (0 by default). Only the lowest 15 bits are used. |

#### See also

| |
|---|
| [RenderableManager.Builder](-builder/blend-order.md) |
| [setGlobalBlendOrderEnabledAt](set-global-blend-order-enabled-at.md) |
