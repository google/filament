//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderTarget](../index.md)/[Builder](index.md)/[layer](layer.md)

# layer

[main]\
open fun [layer](layer.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layer: Int): [RenderTarget.Builder](index.md)

Sets an index of a single layer for 2d array, cubemap array, and 3d textures at the given 

attachment point.

For cubemap array textures, layer is translated into an array index and face according to

- index: layer / 6
- face: layer % 6

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| attachment | The attachment point. |
| layer | The associated cubemap layer. |
