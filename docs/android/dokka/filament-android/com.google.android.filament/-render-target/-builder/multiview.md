//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderTarget](../index.md)/[Builder](index.md)/[multiview](multiview.md)

# multiview

[main]\
open fun [multiview](multiview.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layerCount: Int): [RenderTarget.Builder](index.md)

Sets the starting index of the 2d array textures for multiview at the given attachment point. 

This requires COLOR and DEPTH attachments (if set) to be of 2D array textures.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| attachment | The attachment point. |
| layerCount | The number of layers used for multiview, starting from baseLayer. |

[main]\
open fun [multiview](multiview.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layerCount: Int, baseLayer: Int): [RenderTarget.Builder](index.md)

Sets the starting index of the 2d array textures for multiview at the given attachment point. 

This requires COLOR and DEPTH attachments (if set) to be of 2D array textures.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| attachment | The attachment point. |
| layerCount | The number of layers used for multiview, starting from baseLayer. |
| baseLayer | The starting index of the 2d array texture. |
