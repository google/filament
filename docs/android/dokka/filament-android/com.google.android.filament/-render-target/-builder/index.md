//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderTarget](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Constructs `RenderTarget` objects using a builder pattern.

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [RenderTarget](../index.md)<br>Creates the RenderTarget object and returns a pointer to it. |
| [face](face.md) | [main]<br>open fun [face](face.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), face: [Texture.CubemapFace](../../-texture/-cubemap-face/index.md)): [RenderTarget.Builder](index.md)<br>Sets the cubemap face for a given attachment point. |
| [layer](layer.md) | [main]<br>open fun [layer](layer.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layer: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets the layer for a given attachment point (for 3D textures). |
| [mipLevel](mip-level.md) | [main]<br>open fun [mipLevel](mip-level.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets the mipmap level for a given attachment point. |
| [texture](texture.md) | [main]<br>open fun [texture](texture.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), texture: [Texture](../../-texture/index.md)): [RenderTarget.Builder](index.md)<br>Sets a texture to a given attachment point. |
