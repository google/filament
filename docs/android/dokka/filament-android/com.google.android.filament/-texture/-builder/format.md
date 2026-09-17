//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Texture](../index.md)/[Builder](index.md)/[format](format.md)

# format

[main]\
open fun [format](format.md)(format: [Texture.InternalFormat](../-internal-format/index.md)): [Texture.Builder](index.md)

Specifies the *internal* format of this texture. 

The internal format specifies how texels are stored (which may be different from how they're specified in setImage()). InternalFormat specifies both the color components and the data type used.

#### Return

This Builder, for chaining calls.

#### Parameters

main

| | |
|---|---|
| format | Format of the texture's texel. |

#### See also

| |
|---|
| [Texture.InternalFormat](../-internal-format/index.md) |
| setImage |
