//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setExternalStream](set-external-stream.md)

# setExternalStream

[main]\
open fun [setExternalStream](set-external-stream.md)(engine: [Engine](../-engine/index.md), stream: [Stream](../-stream/index.md))

Specify the external stream to associate with this Texture. Typically, the external 

stream is OS specific, and can be a video or camera stream. There are many restrictions when using an external stream as a texture, such as:

- only the level of detail (lod) 0 can be specified
- only nearest or linear filtering is supported
- the size and format of the texture is defined by the external stream

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to. |
| stream | A Stream object<br>@attention `engine` must be the instance passed to Builder::build() |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |
| [Stream](../-stream/index.md) |
