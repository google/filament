//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setExternalStream](set-external-stream.md)

# setExternalStream

[main]\
open fun [setExternalStream](set-external-stream.md)(engine: [Engine](../-engine/index.md), stream: [Stream](../-stream/index.md))

Specifies the external stream to associate with this `Texture`. 

This `Texture` instance must use [SAMPLER_EXTERNAL](-sampler/-s-a-m-p-l-e-r_-e-x-t-e-r-n-a-l/index.md).

Typically the external image is OS specific, and can be a video or camera frame. There are many restrictions when using an external image as a texture, such as:

- only the level of detail (LOD) 0 can be specified
- only [NEAREST](../-texture-sampler/-mag-filter/-n-e-a-r-e-s-t/index.md) or [LINEAR](../-texture-sampler/-mag-filter/-l-i-n-e-a-r/index.md) filtering is supported
- the size and format of the texture is defined by the external image

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| stream | A [Stream](../-stream/index.md) object |

#### See also

| |
|---|
| [Stream](../-stream/index.md) |
| [Texture.Builder](-builder/sampler.md) |

#### Throws

| | |
|---|---|
| [IllegalStateException](https://developer.android.com/reference/kotlin/java/lang/IllegalStateException.html) | if the sampler type is not [SAMPLER_EXTERNAL](-sampler/-s-a-m-p-l-e-r_-e-x-t-e-r-n-a-l/index.md) |
