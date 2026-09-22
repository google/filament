//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Skybox](index.md)/[setLayerMask](set-layer-mask.md)

# setLayerMask

[main]\
open fun [setLayerMask](set-layer-mask.md)(select: Int, values: Int)

Sets bits in a visibility mask. 

By default, this is 0x1.

This provides a simple mechanism for hiding or showing this Skybox in a Scene.

For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits unaffected, call: `setLayerMask(7, 2)`.

#### Parameters

main

| | |
|---|---|
| select | the set of bits to affect |
| values | the replacement values for the affected bits |

#### See also

| |
|---|
| [View](../-view/set-visible-layers.md) |
