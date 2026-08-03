//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Stream](index.md)

# Stream

open class [Stream](index.md)

`Stream` is used to attach a native video stream to a filament [Texture](../-texture/index.md). Stream supports three different configurations: 

- ACQUIRED
- connects to an Android AHardwareBuffer
- NATIVE
- connects to an Android SurfaceTexture

 Before explaining these different configurations, let's review the high-level structure of an AR or video application that uses Filament. 

```kotlin
while (true) {

    // Misc application work occurs here, such as:
    // - Writing the image data for a video frame into a Stream
    // - Moving the Filament Camera

    if (renderer.beginFrame(swapChain)) {
        renderer.render(view);
        renderer.endFrame();
    }
}

```

 Let's say that the video image data at the time of a particular invocation of beginFrame becomes visible to users at time A. The 3D scene state (including the camera) at the time of that same invocation becomes apparent to users at time B. 

- If time A matches time B, we say that the stream is synchronized.
- Filament invokes low-level graphics commands on the driver thread.
- The thread that calls beginFrame is called the main thread.

 For **ACQUIRED** streams, there is no need to perform the copy because Filament explictly acquires the stream, then releases it later via a callback function. This configuration is especially useful when the Vulkan backend is enabled. 

 For **NATIVE** streams, Filament does not make any synchronization guarantee. However they are simple to use and do not incur a copy. These are often appropriate in video applications. 

 Please see `sample-stream-test` and `sample-hello-camera` for usage examples. 

#### See also

| |
|---|
| [Texture](../-texture/set-external-stream.md) |
| [Engine](../-engine/destroy-stream.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Use `Builder` to construct an Stream object instance. |
| [StreamType](-stream-type/index.md) | [main]<br>enum [StreamType](-stream-type/index.md)<br>Represents the immutable stream type. |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getStreamType](get-stream-type.md) | [main]<br>open fun [getStreamType](get-stream-type.md)(): [Stream.StreamType](-stream-type/index.md)<br>Indicates whether this `Stream` is NATIVE or ACQUIRED. |
| [getTimestamp](get-timestamp.md) | [main]<br>open fun [getTimestamp](get-timestamp.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns the presentation time of the currently displayed frame in nanosecond. |
| [setAcquiredImage](set-acquired-image.md) | [main]<br>open fun [setAcquiredImage](set-acquired-image.md)(hwbuffer: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Updates an ```kotlin ACQUIRED ```  stream with an image that is guaranteed to be used in the next frame. |
| [setDimensions](set-dimensions.md) | [main]<br>open fun [setDimensions](set-dimensions.md)(width: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html), height: [Int](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-int/index.html))<br>Updates the size of the incoming stream. |
