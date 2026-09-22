//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[StreamHelper](index.md)

# StreamHelper

[main]\
class [StreamHelper](index.md)

## Functions

| Name | Summary |
|---|---|
| [setAcquiredImage](set-acquired-image.md) | [main]<br>open fun [setAcquiredImage](set-acquired-image.md)(stream: [Stream](../../com.google.android.filament/-stream/index.md), hwbuffer: HardwareBuffer, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))<br>Updates an ACQUIRED stream with a HardwareBuffer image.<br>[main]<br>open fun [setAcquiredImage](set-acquired-image.md)(stream: [Stream](../../com.google.android.filament/-stream/index.md), hwbuffer: HardwareBuffer, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html), transform: Array&lt;Float&gt;)<br>Updates an ACQUIRED stream with a HardwareBuffer image and an optional 3x3 transform matrix. |
| [setStreamSource](set-stream-source.md) | [main]<br>open fun [setStreamSource](set-stream-source.md)(builder: [Stream.Builder](../../com.google.android.filament/-stream/-builder/index.md), surfaceTexture: SurfaceTexture): [Stream.Builder](../../com.google.android.filament/-stream/-builder/index.md)<br>Associates a SurfaceTexture with a [Stream.Builder](../../com.google.android.filament/-stream/-builder/index.md) to create a NATIVE stream. |
