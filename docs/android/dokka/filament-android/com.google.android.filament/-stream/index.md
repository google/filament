//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Stream](index.md)

# Stream

open class [Stream](index.md)

Stream is used to attach a video stream to a Filament `Texture`. 

Note that the `Stream` class is fairly Android centric. It supports two different configurations:

- ACQUIRED.....connects to an Android AHardwareBuffer
- NATIVE.......connects to an Android SurfaceTexture Before explaining these different configurations, let's review the high-level structure of an AR or video application that uses Filament:

```kotlin

while (true) {

    // Misc application work occurs here, such as:
    // - Writing the image data for a video frame into a Stream
    // - Moving the Filament Camera

    if (renderer->beginFrame(swapChain)) {
        renderer->render(view);
        renderer->endFrame();
    }
}

```

Let's say that the video image data at the time of a particular invocation of `beginFrame` becomes visible to users at time A. The 3D scene state (including the camera) at the time of that same invocation becomes apparent to users at time B.

- If time A matches time B, we say that the stream is \em{synchronized}.
- Filament invokes low-level graphics commands on the \em{driver thread}.
- The thread that calls `beginFrame` is called the \em{main thread}. For ACQUIRED streams, there is no need to perform the copy because Filament explicitly acquires the stream, then releases it later via a callback function. This configuration is especially useful when the Vulkan backend is enabled. NATIVE streams are deprecated because they are backend specific and do not make any synchronization guarantee. Please see `sample-stream-test` and `sample-hello-camera` for usage examples.

#### See also

| |
|---|
| backend.StreamType |
| [Texture](../-texture/set-external-stream.md) |
| [Engine](../-engine/destroy-stream.md) |

## Types

| Name | Summary |
|---|---|
| [Builder](-builder/index.md) | [main]<br>open class [Builder](-builder/index.md)<br>Constructs a Stream object instance. |
| [StreamType](-stream-type/index.md) | [main]<br>enum [StreamType](-stream-type/index.md)<br>Stream for external textures |

## Functions

| Name | Summary |
|---|---|
| [getNativeObject](get-native-object.md) | [main]<br>open fun [getNativeObject](get-native-object.md)(): Long |
| [getStreamType](get-stream-type.md) | [main]<br>open fun [getStreamType](get-stream-type.md)(): [Stream.StreamType](-stream-type/index.md)<br>Indicates whether this stream is a NATIVE stream or ACQUIRED stream. |
| [getTimestamp](get-timestamp.md) | [main]<br>open fun [getTimestamp](get-timestamp.md)(): Long<br>Returns the presentation time of the currently displayed frame in nanosecond. |
| [setDimensions](set-dimensions.md) | [main]<br>open fun [setDimensions](set-dimensions.md)(width: Int, height: Int)<br>Updates the size of the incoming stream. |
| [wrap](wrap.md) | [main]<br>open fun [wrap](wrap.md)(nativeObject: Long): [Stream](index.md) |
