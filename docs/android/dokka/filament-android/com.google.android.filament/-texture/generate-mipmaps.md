//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[generateMipmaps](generate-mipmaps.md)

# generateMipmaps

[main]\
open fun [generateMipmaps](generate-mipmaps.md)(engine: [Engine](../-engine/index.md))

Generates all the mipmap levels automatically. 

This requires the texture to have a color-renderable format and usage set to BLIT_SRC | BLIT_DST. If unspecified, usage bits are set automatically.

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to.<br>@attention `engine` must be the instance passed to Builder::build() |
