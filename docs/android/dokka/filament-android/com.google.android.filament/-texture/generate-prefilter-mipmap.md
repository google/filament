//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[generatePrefilterMipmap](generate-prefilter-mipmap.md)

# generatePrefilterMipmap

[main]\
open fun [generatePrefilterMipmap](generate-prefilter-mipmap.md)(engine: [Engine](../-engine/index.md), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md), faceOffsetsInBytes: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html)&gt;, options: [Texture.PrefilterOptions](-prefilter-options/index.md))

Creates a reflection map from an environment map. 

This is a utility function that replaces calls to setImage. The provided environment map is processed and all mipmap levels are populated. The processing is similar to the offline tool `cmgen` at a lower quality setting.

This function is intended to be used when the environment cannot be processed offline, for instance if it's generated at runtime.

The source data must obey to some constraints:

- the data [format](-format/index.md) must be [RGB](-format/-r-g-b/index.md)
- the data [type](-type/index.md) must be one of 
   
   - 
      [FLOAT](-type/-f-l-o-a-t/index.md)
   - 
      [HALF](-type/-h-a-l-f/index.md)

The current texture must be a cubemap.

The reflections cubemap's [internal format](-internal-format/index.md) cannot be a compressed format.

The reflections cubemap's dimension must be a power-of-two.

This operation is computationally intensive, especially with large environments and is currently **synchronous**. Expect about 1ms for a 16 × 16 cubemap.

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| buffer | Client-side buffer containing the image to set. `buffer`'s [format](-format/index.md) and [type](-type/index.md) must match the constraints above. |
| faceOffsetsInBytes | Offsets in bytes into `buffer` for all six images. The offsets are specified in the following order: +x, -x, +y, -y, +z, -z. |
| options | Optional parameter to control user-specified quality and options. <br>`faceOffsetsInBytes` are offsets in byte in the `buffer` relative to the current [position](https://developer.android.com/reference/kotlin/java/nio/Buffer.html#position). Use [CubemapFace](-cubemap-face/index.md) to index the `faceOffsetsInBytes` array. All six faces must be tightly packed. |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |
