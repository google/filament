//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[external](external.md)

# external

[main]\
open fun [external](external.md)(): [Texture.Builder](index.md)

Creates an external texture. The content must be set using setExternalImage(). The sampler can be SAMPLER_EXTERNAL or SAMPLER_2D depending on the format. Generally YUV formats must use SAMPLER_EXTERNAL. This depends on the backend features and is not validated.

#### Return

This Builder, for chaining calls.
