//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderTarget](../index.md)/[Builder](index.md)/[texture](texture.md)

# texture

[main]\
open fun [texture](texture.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), texture: [Texture](../../-texture/index.md)): [RenderTarget.Builder](index.md)

Sets a texture to a given attachment point. 

When using a DEPTH attachment, it is important to always disable post-processing in the View. Failing to do so will cause the DEPTH attachment to be ignored in most cases.

When the intention is to keep the content of the DEPTH attachment after rendering, Usage::SAMPLEABLE must be set on the DEPTH attachment, otherwise the content of the DEPTH buffer may be discarded.

#### Return

A reference to this Builder for chaining calls.

#### Parameters

main

| | |
|---|---|
| attachment | The attachment point of the texture. |
| texture | The associated texture object. |
