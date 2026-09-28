//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[RenderTarget](../index.md)/[Builder](index.md)

# Builder

[main]\
open class [Builder](index.md)

Use Builder to construct a RenderTarget object instance

## Constructors

| | |
|---|---|
| [Builder](-builder.md) | [main]<br>constructor() |

## Functions

| Name | Summary |
|---|---|
| [build](build.md) | [main]<br>open fun [build](build.md)(engine: [Engine](../../-engine/index.md)): [RenderTarget](../index.md)<br>Creates the RenderTarget object and returns a pointer to it. |
| [face](face.md) | [main]<br>open fun [face](face.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), face: [RenderTarget.CubemapFace](../-cubemap-face/index.md)): [RenderTarget.Builder](index.md)<br>Sets the face for cubemap textures at the given attachment point. |
| [layer](layer.md) | [main]<br>open fun [layer](layer.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layer: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets an index of a single layer for 2d array, cubemap array, and 3d textures at the given attachment point. |
| [mipLevel](mip-level.md) | [main]<br>open fun [mipLevel](mip-level.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets the mipmap level for a given attachment point. |
| [multiview](multiview.md) | [main]<br>open fun [multiview](multiview.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layerCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>open fun [multiview](multiview.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), layerCount: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), baseLayer: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets the starting index of the 2d array textures for multiview at the given attachment point. |
| [samples](samples.md) | [main]<br>open fun [samples](samples.md)(samples: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [RenderTarget.Builder](index.md)<br>Sets the number of samples used for MSAA (Multisample Anti-Aliasing). |
| [texture](texture.md) | [main]<br>open fun [texture](texture.md)(attachment: [RenderTarget.AttachmentPoint](../-attachment-point/index.md), texture: [Texture](../../-texture/index.md)): [RenderTarget.Builder](index.md)<br>Sets a texture to a given attachment point. |
