//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[depth](depth.md)

# depth

[main]\
open fun [depth](depth.md)(depth: Int): [Texture.Builder](index.md)

Specifies the depth in texels of the texture. 

Doesn't need to be a power-of-two. The depth controls the number of layers in a 2D array texture. Values greater than 1 effectively create a 3D texture.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| depth | Depth of the texture in texels (default: 1). |
