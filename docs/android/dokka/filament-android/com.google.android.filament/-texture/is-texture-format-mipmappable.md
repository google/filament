//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[isTextureFormatMipmappable](is-texture-format-mipmappable.md)

# isTextureFormatMipmappable

[main]\
open fun [isTextureFormatMipmappable](is-texture-format-mipmappable.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Checks whether a given texture format is supported for mipmapping in this [Engine](../-engine/index.md). This depends on the selected backend.

#### Return

`true` if this format is supported for texturing.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) to test the [InternalFormat](-internal-format/index.md) against |
| format | format to check |
