//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[generateMipmaps](generate-mipmaps.md)

# generateMipmaps

[main]\
open fun [generateMipmaps](generate-mipmaps.md)(engine: [Engine](../-engine/index.md))

Generates all the mipmap levels automatically. This requires the texture to have a color-renderable format. 

This `Texture` instance must **not** use [SAMPLER_CUBEMAP](-sampler/-s-a-m-p-l-e-r_-c-u-b-e-m-a-p/index.md), or it has no effect.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
