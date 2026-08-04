//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setExternalImage](set-external-image.md)

# setExternalImage

[main]\
open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), eglImage: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Specifies the external image to associate with this `Texture`. 

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
| eglImage | An opaque handle to a platform specific image. Supported types are `eglImageOES` on Android and `CVPixelBufferRef` on iOS. <br>On iOS the following pixel formats are supported: <br>-     `kCVPixelFormatType_32BGRA` -     `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange` |

#### See also

| |
|---|
| [Texture.Builder](-builder/sampler.md) |

[main]\
open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), externalImageRef: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html))

Specifies the external image to associate with this `Texture`. 

Typically, the external image is OS specific, and can be a video or camera frame. There are many restrictions when using an external image as a texture, such as:

- only the level of detail (lod) 0 can be specified
- only nearest or linear filtering is supported
- the size and format of the texture is defined by the external image
- only the CLAMP_TO_EDGE wrap mode is supported

#### Parameters

main

| | |
|---|---|
| engine | [Engine](../-engine/index.md) this texture is associated to. Must be the instance passed to [Builder.build()](-builder/build.md). |
| externalImageRef | An OS specific Object. On Android it must be a `android.hardware.HardwareBuffer` |
