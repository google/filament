//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderTarget](index.md)

# RenderTarget

open class [RenderTarget](index.md)

An offscreen render target that can be associated with a [View](../-view/index.md) and contains weak references to a set of attached [Texture](../-texture/index.md) objects. 

 Clients are responsible for the lifetime of all associated `Texture` attachments. 

#### See also

| |
|---|
| [View](../-view/index.md) |

## Types

| Name | Summary |
|---|---|
| [AttachmentPoint](-attachment-point/index.md) | [main]<br>enum [AttachmentPoint](-attachment-point/index.md)<br>An attachment point is a slot that can be assigned to a [Texture](../-texture/index.md). |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Constructs `RenderTarget` objects using a builder pattern. |

## Functions

| Name | Summary |
|---|---|
| [getFace](get-face.md) | [main]<br>open fun [getFace](get-face.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [Texture.CubemapFace](../-texture/-cubemap-face/index.md)<br>Returns the face of a cubemap set on the given attachment point. |
| [getLayer](get-layer.md) | [main]<br>open fun [getLayer](get-layer.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the texture-layer set on the given attachment point. |
| [getMipLevel](get-mip-level.md) | [main]<br>open fun [getMipLevel](get-mip-level.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the mipmap level set on the given attachment point. |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTexture](get-texture.md) | [main]<br>open fun [getTexture](get-texture.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [Texture](../-texture/index.md)<br>Gets the texture set on the given attachment point. |
