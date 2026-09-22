//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)

# Texture

[main]\
open class [Texture](index.md)

Texture 

The Texture class supports:

- 2D textures
- 3D textures
- Cube maps
- mip mapping

# Creation and destruction

A Texture object is created using the Texture::Builder and destroyed by calling Engine::destroy(const Texture*).

```kotlin

 filament::Engine* engine = filament::Engine::create();

 filament::Texture* texture = filament::Texture::Builder()
             .width(64)
             .height(64)
             .build(*engine);

 engine->destroy(texture);

```

## Types

| Name | Summary |
|---|---|
| [AsyncCallStatus](-async-call-status/index.md) | [main]<br>enum [AsyncCallStatus](-async-call-status/index.md)<br>Outcome of an asynchronous operation, reported to its completion callback. |
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use Builder to construct a Texture object instance |
| [CompressedType](-compressed-type/index.md) | [main]<br>enum [CompressedType](-compressed-type/index.md)<br>Compressed pixel data types |
| [CubemapFace](-cubemap-face/index.md) | [main]<br>enum [CubemapFace](-cubemap-face/index.md)<br>Texture Cubemap Face |
| [Format](-format/index.md) | [main]<br>enum [Format](-format/index.md)<br>Pixel Data Format |
| [InternalFormat](-internal-format/index.md) | [main]<br>enum [InternalFormat](-internal-format/index.md)<br>Supported texel formats These formats are typically used to specify a texture's internal storage format. |
| [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) | [main]<br>open class [~~PixelBufferDescriptor~~](-pixel-buffer-descriptor/index.md) : [PixelBufferDescriptor](../-pixel-buffer-descriptor/index.md) |
| [Sampler](-sampler/index.md) | [main]<br>enum [Sampler](-sampler/index.md)<br>Texture sampler type |
| [Swizzle](-swizzle/index.md) | [main]<br>enum [Swizzle](-swizzle/index.md)<br>Texture swizzle |
| [Type](-type/index.md) | [main]<br>enum [Type](-type/index.md)<br>Pixel Data Type |
| [Usage](-usage/index.md) | [main]<br>open class [Usage](-usage/index.md)<br>Bitmask describing the intended Texture Usage |

## Properties

| Name | Summary |
|---|---|
| [BASE_LEVEL](-b-a-s-e_-l-e-v-e-l.md) | [main]<br>val [BASE_LEVEL](-b-a-s-e_-l-e-v-e-l.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 0 |

## Functions

| Name | Summary |
|---|---|
| [computeTextureDataSize](compute-texture-data-size.md) | [main]<br>open fun [computeTextureDataSize](compute-texture-data-size.md)(format: [Texture.Format](-format/index.md), type: [Texture.Type](-type/index.md), stride: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), alignment: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [generateMipmaps](generate-mipmaps.md) | [main]<br>open fun [generateMipmaps](generate-mipmaps.md)(engine: [Engine](../-engine/index.md))<br>Generates all the mipmap levels automatically. |
| [getDepth](get-depth.md) | [main]<br>open fun [getDepth](get-depth.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>open fun [getDepth](get-depth.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the depth of a 3D texture level |
| [getFormat](get-format.md) | [main]<br>open fun [getFormat](get-format.md)(): [Texture.InternalFormat](-internal-format/index.md)<br>Return this texture InternalFormat as set by Builder::format(). |
| [getHeight](get-height.md) | [main]<br>open fun [getHeight](get-height.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>open fun [getHeight](get-height.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the height of a 2D or 3D texture level |
| [getLevels](get-levels.md) | [main]<br>open fun [getLevels](get-levels.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the maximum number of levels this texture can have. |
| [getMaxArrayTextureLayers](get-max-array-texture-layers.md) | [main]<br>open fun [getMaxArrayTextureLayers](get-max-array-texture-layers.md)(engine: [Engine](../-engine/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getMaxTextureSize](get-max-texture-size.md) | [main]<br>open fun [getMaxTextureSize](get-max-texture-size.md)(engine: [Engine](../-engine/index.md), type: [Texture.Sampler](-sampler/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTarget](get-target.md) | [main]<br>open fun [getTarget](get-target.md)(): [Texture.Sampler](-sampler/index.md)<br>Return this texture Sampler as set by Builder::sampler(). |
| [getWidth](get-width.md) | [main]<br>open fun [getWidth](get-width.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>open fun [getWidth](get-width.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Returns the width of a 2D or 3D texture level |
| [isCreationComplete](is-creation-complete.md) | [main]<br>open fun [isCreationComplete](is-creation-complete.md)(): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>This non-blocking method checks if the resource has finished creation *successfully*. |
| [isProtectedTexturesSupported](is-protected-textures-supported.md) | [main]<br>open fun [isProtectedTexturesSupported](is-protected-textures-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isTextureFormatCompressed](is-texture-format-compressed.md) | [main]<br>open fun [isTextureFormatCompressed](is-texture-format-compressed.md)(format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isTextureFormatMipmappable](is-texture-format-mipmappable.md) | [main]<br>open fun [isTextureFormatMipmappable](is-texture-format-mipmappable.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isTextureFormatSupported](is-texture-format-supported.md) | [main]<br>open fun [isTextureFormatSupported](is-texture-format-supported.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [isTextureSwizzleSupported](is-texture-swizzle-supported.md) | [main]<br>open fun [isTextureSwizzleSupported](is-texture-swizzle-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [setExternalImage](set-external-image.md) | [main]<br>open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), image: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Specify the external image to associate with this Texture.<br>[main]<br>open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), image: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), plane: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Specify the external image and plane to associate with this Texture. |
| [setExternalStream](set-external-stream.md) | [main]<br>open fun [setExternalStream](set-external-stream.md)(engine: [Engine](../-engine/index.md), stream: [Stream](../-stream/index.md))<br>Specify the external stream to associate with this Texture. |
| [setImage](set-image.md) | [main]<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>inline helper to update a 2D texture<br>[main]<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), zoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>Updates a sub-image of a 3D texture or 2D texture array for a level. |
| [validatePixelFormatAndType](validate-pixel-format-and-type.md) | [main]<br>open fun [validatePixelFormatAndType](validate-pixel-format-and-type.md)(internalFormat: [Texture.InternalFormat](-internal-format/index.md), format: [Texture.Format](-format/index.md), type: [Texture.Type](-type/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html) |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)): [Texture](index.md) |
