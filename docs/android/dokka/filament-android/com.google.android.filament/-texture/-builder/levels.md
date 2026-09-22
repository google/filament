//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[levels](levels.md)

# levels

[main]\
open fun [levels](levels.md)(levels: Int): [Texture.Builder](index.md)

Specifies the numbers of mip map levels. 

This creates a mip-map pyramid. The maximum number of levels a texture can have is such that max(width, height, level) / 2^MAX_LEVELS = 1

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| levels | Number of mipmap levels for this texture. |
