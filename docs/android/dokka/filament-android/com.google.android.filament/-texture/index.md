//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)

# Texture

open class [Texture](index.md)

Texture 

The `Texture` class supports:

- 2D textures
- 3D textures
- Cube maps
- mip mapping

# Usage example

 A `Texture` object is created using the [Texture.Builder](-builder/index.md) and destroyed by calling [destroyTexture](../-engine/destroy-texture.md). They're bound using setParameter. ```kotlin
 Engine engine = Engine.create();

 Material material = new Material.Builder()
             .payload( ... )
             .build(ending);

 MaterialInstance mi = material.getDefaultInstance();

 Texture texture = new Texture.Builder()
             .width(64)
             .height(64)
             .build(engine);

 texture.setImage(engine, 0,
         new Texture.PixelBufferDescriptor( ... ));

 mi.setParameter("parameterName", texture, new TextureSampler());

```

#### See also

| |
|---|
| setImage |
| [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) |
| [MaterialInstance](../-material-instance/set-parameter.md) |

## Constructors

| | |
|---|---|
| [Texture](-texture.md) | [main]<br>constructor(nativeTexture: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use `Builder` to construct a `Texture` object instance. |
| [CompressedFormat](-compressed-format/index.md) | [main]<br>enum [CompressedFormat](-compressed-format/index.md)<br>Compressed data types for use with [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) |
| [CubemapFace](-cubemap-face/index.md) | [main]<br>enum [CubemapFace](-cubemap-face/index.md)<br>Cubemap faces |
| [Format](-format/index.md) | [main]<br>enum [Format](-format/index.md)<br>Pixel color format |
| [InternalFormat](-internal-format/index.md) | [main]<br>enum [InternalFormat](-internal-format/index.md)<br>Internal texel formats These formats are used to specify a texture's internal storage format. |
| [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) | [main]<br>open class [PixelBufferDescriptor](-pixel-buffer-descriptor/index.md)<br>A descriptor to an image in main memory, typically used to transfer image data from the CPU to the GPU. |
| [PrefilterOptions](-prefilter-options/index.md) | [main]<br>open class [PrefilterOptions](-prefilter-options/index.md)<br>Options of [generatePrefilterMipmap](generate-prefilter-mipmap.md) |
| [Sampler](-sampler/index.md) | [main]<br>enum [Sampler](-sampler/index.md)<br>Type of sampler |
| [Swizzle](-swizzle/index.md) | [main]<br>enum [Swizzle](-swizzle/index.md)<br>Texture swizzling channels |
| [Type](-type/index.md) | [main]<br>enum [Type](-type/index.md)<br>Pixel data type |
| [Usage](-usage/index.md) | [main]<br>open class [Usage](-usage/index.md)<br>A bitmask to specify how the texture will be used. |

## Properties

| Name | Summary |
|---|---|
| [BASE_LEVEL](-b-a-s-e_-l-e-v-e-l.md) | [main]<br>val [BASE_LEVEL](-b-a-s-e_-l-e-v-e-l.md): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) = 0 |

## Functions

| Name | Summary |
|---|---|
| [generateMipmaps](generate-mipmaps.md) | [main]<br>open fun [generateMipmaps](generate-mipmaps.md)(engine: [Engine](../-engine/index.md))<br>Generates all the mipmap levels automatically. |
| [generatePrefilterMipmap](generate-prefilter-mipmap.md) | [main]<br>open fun [generatePrefilterMipmap](generate-prefilter-mipmap.md)(engine: [Engine](../-engine/index.md), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md), faceOffsetsInBytes: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;, options: [Texture.PrefilterOptions](-prefilter-options/index.md))<br>Creates a reflection map from an environment map. |
| [getDepth](get-depth.md) | [main]<br>open fun [getDepth](get-depth.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Queries the number of layers of given level of this texture has. |
| [getFormat](get-format.md) | [main]<br>open fun [getFormat](get-format.md)(): [Texture.InternalFormat](-internal-format/index.md) |
| [getHeight](get-height.md) | [main]<br>open fun [getHeight](get-height.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Queries the height of a given level of this texture. |
| [getLevels](get-levels.md) | [main]<br>open fun [getLevels](get-levels.md)(): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getMaxArrayTextureLayers](get-max-array-texture-layers.md) | [main]<br>open fun [getMaxArrayTextureLayers](get-max-array-texture-layers.md)(engine: [Engine](../-engine/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getMaxTextureSize](get-max-texture-size.md) | [main]<br>open fun [getMaxTextureSize](get-max-texture-size.md)(engine: [Engine](../-engine/index.md), type: [Texture.Sampler](-sampler/index.md)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html) |
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTarget](get-target.md) | [main]<br>open fun [getTarget](get-target.md)(): [Texture.Sampler](-sampler/index.md) |
| [getWidth](get-width.md) | [main]<br>open fun [getWidth](get-width.md)(level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)): [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)<br>Queries the width of a given level of this texture. |
| [isTextureFormatMipmappable](is-texture-format-mipmappable.md) | [main]<br>open fun [isTextureFormatMipmappable](is-texture-format-mipmappable.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks whether a given texture format is supported for mipmapping in this [Engine](../-engine/index.md). |
| [isTextureFormatSupported](is-texture-format-supported.md) | [main]<br>open fun [isTextureFormatSupported](is-texture-format-supported.md)(engine: [Engine](../-engine/index.md), format: [Texture.InternalFormat](-internal-format/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks whether a given format is supported for texturing in this [Engine](../-engine/index.md). |
| [isTextureSwizzleSupported](is-texture-swizzle-supported.md) | [main]<br>open fun [isTextureSwizzleSupported](is-texture-swizzle-supported.md)(engine: [Engine](../-engine/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks whether texture swizzling is supported in this [Engine](../-engine/index.md). |
| [setExternalImage](set-external-image.md) | [main]<br>open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), externalImageRef: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html))<br>open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), eglImage: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))<br>Specifies the external image to associate with this `Texture`. |
| [setExternalStream](set-external-stream.md) | [main]<br>open fun [setExternalStream](set-external-stream.md)(engine: [Engine](../-engine/index.md), stream: [Stream](../-stream/index.md))<br>Specifies the external stream to associate with this `Texture`. |
| [setImage](set-image.md) | [main]<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>`setImage` is used to modify the whole content of the texture from a CPU-buffer.<br>[main]<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>`setImage` is used to modify a sub-region of the texture from a CPU-buffer.<br>[main]<br>open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), zoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))<br>`setImage` is used to modify a sub-region of a 3D texture, 2D texture array or cubemap from a CPU-buffer. |
| [validatePixelFormatAndType](validate-pixel-format-and-type.md) | [main]<br>open fun [validatePixelFormatAndType](validate-pixel-format-and-type.md)(internalFormat: [Texture.InternalFormat](-internal-format/index.md), pixelDataFormat: [Texture.Format](-format/index.md), pixelDataType: [Texture.Type](-type/index.md)): [Boolean](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-boolean/index.html)<br>Checks whether a given combination of texture format, pixel data and type is valid. |
