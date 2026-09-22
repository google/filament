//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[TextureHelper](index.md)/[setExternalImage](set-external-image.md)

# setExternalImage

[main]\
open fun [setExternalImage](set-external-image.md)(engine: [Engine](../../com.google.android.filament/-engine/index.md), texture: [Texture](../../com.google.android.filament/-texture/index.md), buffer: HardwareBuffer): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)

Specify the external image to associate with this Texture from an Android HardwareBuffer.

#### Return

true if successful, false otherwise.

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to. |
| texture | Texture to associate the image with. |
| buffer | An Android HardwareBuffer. |
