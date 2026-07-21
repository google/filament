//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderableManager](../index.md)/[Builder](index.md)/[layerMask](layer-mask.md)

# layerMask

[main]\
open fun [layerMask](layer-mask.md)(select: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), value: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderableManager.Builder](index.md)

Sets bits in a visibility mask. By default, this is 0x1. 

This feature provides a simple mechanism for hiding and showing groups of renderables in a Scene. See [setVisibleLayers](../../-view/set-visible-layers.md).

For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits unaffected, do: `builder.layerMask(7, 2)`.

#### Parameters

main

| | |
|---|---|
| select | the set of bits to affect |
| value | the replacement values for the affected bits |

#### See also

| |
|---|
| [RenderableManager](../set-layer-mask.md) |
