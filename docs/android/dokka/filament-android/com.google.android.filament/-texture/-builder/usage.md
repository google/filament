//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[usage](usage.md)

# usage

[main]\
open fun [usage](usage.md)(usage: Int): [Texture.Builder](index.md)

Specifies if the texture will be used as a render target attachment. 

If the texture is potentially rendered into, it may require a different memory layout, which needs to be known during construction.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| usage | Defaults to Texture::Usage::DEFAULT; c.f. Texture::Usage::COLOR_ATTACHMENT. |
