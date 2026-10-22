//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Texture](index.md)/[setExternalImage](set-external-image.md)

# setExternalImage

[main]\
open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), image: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Specify the external image to associate with this Texture. Typically, the external 

image is OS specific, and can be a video or camera frame. There are many restrictions when using an external image as a texture, such as:

- only the level of detail (lod) 0 can be specified
- only nearest or linear filtering is supported
- the size and format of the texture is defined by the external image
- only the CLAMP_TO_EDGE wrap mode is supported

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to. |
| image | An opaque handle to a platform specific image. It must be created using Platform specific APIs. For example PlatformEGL::createExternalImage(EGLImageKHR eglImage) |

#### See also

| |
|---|
| PlatformEGL#createExternalImage |
| PlatformEGLAndroid#createExternalImage |
| PlatformCocoaGL#createExternalImage |
| PlatformCocoaTouchGL#createExternalImage |

[main]\
open fun [setExternalImage](set-external-image.md)(engine: [Engine](../-engine/index.md), image: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), plane: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))

Specify the external image and plane to associate with this Texture. Typically, the external 

image is OS specific, and can be a video or camera frame. When using this method, the external image must be a planar type (such as a YUV camera frame). The plane parameter selects which image plane is bound to this texture.

A single external image can be bound to different Filament textures, with each texture associated with a separate plane:

```kotlin

textureA->setExternalImage(engine, image, 0);
textureB->setExternalImage(engine, image, 1);

```

There are many restrictions when using an external image as a texture, such as:

- only the level of detail (lod) 0 can be specified
- only nearest or linear filtering is supported
- the size and format of the texture is defined by the external image
- only the CLAMP_TO_EDGE wrap mode is supported

#### Parameters

main

| | |
|---|---|
| engine | Engine this texture is associated to. |
| image | An opaque handle to a platform specific image. Supported types are eglImageOES on Android and CVPixelBufferRef on iOS. |
| plane | The plane index of the external image to associate with this texture.<br>This method is only meaningful on iOS with kCVPixelFormatType_420YpCbCr8BiPlanarFullRange images. On platforms other than iOS, this method is a no-op. |
