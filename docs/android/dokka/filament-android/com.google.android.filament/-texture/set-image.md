//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setImage](set-image.md)

# setImage

[main]\
open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

`setImage` is used to modify the whole content of the texture from a CPU-buffer. 

This `Texture` instance must use [SAMPLER_2D](-sampler/-s-a-m-p-l-e-r_2-d/index.md) or [SAMPLER_EXTERNAL](-sampler/-s-a-m-p-l-e-r_-e-x-t-e-r-n-a-l/index.md). If the later is specified and external textures are supported by the driver implementation, this method will have no effect, otherwise it will behave as if the texture was specified with [SAMPLER_2D](-sampler/-s-a-m-p-l-e-r_2-d/index.md).

 This is equivalent to calling: `setImage(engine, level, 0, 0, getWidth(level), getHeight(level), buffer)`

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| level | Level to set the image for. Must be less than [getLevels](get-levels.md). |
| buffer | Client-side buffer containing the image to set. `buffer`'s [format](-format/index.md) must match that of [getFormat](get-format.md) |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |
| [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |

[main]\
open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

`setImage` is used to modify a sub-region of the texture from a CPU-buffer. 

This `Texture` instance must use [SAMPLER_2D](-sampler/-s-a-m-p-l-e-r_2-d/index.md) or [SAMPLER_EXTERNAL](-sampler/-s-a-m-p-l-e-r_-e-x-t-e-r-n-a-l/index.md). If the later is specified and external textures are supported by the driver implementation, this method will have no effect, otherwise it will behave as if the texture was specified with [SAMPLER_2D](-sampler/-s-a-m-p-l-e-r_2-d/index.md).

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| level | Level to set the image for. Must be less than [getLevels](get-levels.md). |
| xoffset | x-offset in texel of the region to modify |
| yoffset | y-offset in texel of the region to modify |
| width | width in texel of the region to modify |
| height | height in texel of the region to modify |
| buffer | Client-side buffer containing the image to set. `buffer`'s [format](-format/index.md) must match that of [getFormat](get-format.md) |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |
| [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |

[main]\
open fun [setImage](set-image.md)(engine: [Engine](../-engine/index.md), level: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), xoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), yoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), zoffset: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), depth: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), buffer: [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md))

`setImage` is used to modify a sub-region of a 3D texture, 2D texture array or cubemap from a CPU-buffer. Cubemaps are treated like a 2D array of six layers. 

This `Texture` instance must use [SAMPLER_2D_ARRAY](-sampler/-s-a-m-p-l-e-r_2-d_-a-r-r-a-y/index.md), [SAMPLER_3D](-sampler/-s-a-m-p-l-e-r_3-d/index.md) or [SAMPLER_CUBEMAP](-sampler/-s-a-m-p-l-e-r_-c-u-b-e-m-a-p/index.md).

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| level | Level to set the image for. Must be less than [getLevels](get-levels.md). |
| xoffset | x-offset in texel of the region to modify |
| yoffset | y-offset in texel of the region to modify |
| zoffset | z-offset in texel of the region to modify |
| width | width in texel of the region to modify |
| height | height in texel of the region to modify |
| depth | depth in texel or index of the region to modify |
| buffer | Client-side buffer containing the image to set. `buffer`'s [format](-format/index.md) must match that of [getFormat](get-format.md) |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |
| [Texture.PixelBufferDescriptor](-pixel-buffer-descriptor/index.md) |

#### Throws

| | |
|---|---|
| [BufferOverflowException](https://developer.android.com/reference/kotlin/java/nio/BufferOverflowException.html) | if the specified parameters would result in reading outside of `buffer`. |
