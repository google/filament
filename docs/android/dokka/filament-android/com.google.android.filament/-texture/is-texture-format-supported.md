//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[isTextureFormatSupported](is-texture-format-supported.md)

# isTextureFormatSupported

[main]\
open fun [isTextureFormatSupported](is-texture-format-supported.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Checks whether a given format is supported for texturing in this [Engine](../-engine/index.md). This depends on the selected backend.

#### Return

`true` if this format is supported for texturing.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) to test the [InternalFormat](-internal-format/index.md) against |
| format | format to check |
