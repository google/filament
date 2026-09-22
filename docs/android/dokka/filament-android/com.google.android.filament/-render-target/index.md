//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[RenderTarget](index.md)

# RenderTarget

open class [RenderTarget](index.md)

An offscreen render target that can be associated with a View and contains weak references to a set of attached Texture objects. 

RenderTarget is intended to be used with the View's post-processing disabled for the most part. especially when a DEPTH attachment is also used (see Builder::texture()).

Custom RenderTarget are ultimately intended to render into textures that might be used during the main render pass.

Clients are responsible for the lifetime of all associated Texture attachments.

#### See also

| |
|---|
| [View](../-view/index.md) |

## Types

| Name | Summary |
|---|---|
| [AttachmentPoint](-attachment-point/index.md) | [main]<br>enum [AttachmentPoint](-attachment-point/index.md)<br>Attachment identifiers |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use Builder to construct a RenderTarget object instance |
| [CubemapFace](-cubemap-face/index.md) | [main]<br>enum [CubemapFace](-cubemap-face/index.md)<br>Texture Cubemap Face |

## Properties

| Name | Summary |
|---|---|
| [MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT](-m-a-x_-s-u-p-p-o-r-t-e-d_-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t-s_-c-o-u-n-t.md) | [main]<br>val [MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT](-m-a-x_-s-u-p-p-o-r-t-e-d_-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t-s_-c-o-u-n-t.md): Int = 8<br>Maximum number of color attachment supported |
| [MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT](-m-i-n_-s-u-p-p-o-r-t-e-d_-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t-s_-c-o-u-n-t.md) | [main]<br>val [MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT](-m-i-n_-s-u-p-p-o-r-t-e-d_-c-o-l-o-r_-a-t-t-a-c-h-m-e-n-t-s_-c-o-u-n-t.md): Int = 4<br>Minimum number of color attachment supported |

## Functions

| Name | Summary |
|---|---|
| [getFace](get-face.md) | [main]<br>open fun [getFace](get-face.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [RenderTarget.CubemapFace](-cubemap-face/index.md)<br>Returns the face of a cubemap set on the given attachment point |
| [getLayer](get-layer.md) | [main]<br>open fun [getLayer](get-layer.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): Int<br>Returns the texture-layer set on the given attachment point |
| [getMipLevel](get-mip-level.md) | [main]<br>open fun [getMipLevel](get-mip-level.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): Int<br>Returns the mipmap level set on the given attachment point |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getSupportedColorAttachmentsCount](get-supported-color-attachments-count.md) | [main]<br>open fun [getSupportedColorAttachmentsCount](get-supported-color-attachments-count.md)(): Int<br>Returns the number of color attachments usable by this instance of Engine. |
| [getTexture](get-texture.md) | [main]<br>open fun [getTexture](get-texture.md)(attachment: [RenderTarget.AttachmentPoint](-attachment-point/index.md)): [Texture](../-texture/index.md)<br>Gets the texture set on the given attachment point |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [RenderTarget](index.md) |
