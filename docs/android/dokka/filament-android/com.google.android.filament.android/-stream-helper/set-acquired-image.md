//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[StreamHelper](index.md)/[setAcquiredImage](set-acquired-image.md)

# setAcquiredImage

[main]\
open fun [setAcquiredImage](set-acquired-image.md)(stream: [Stream](../../com.google.android.filament/-stream/index.md), hwbuffer: HardwareBuffer, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Updates an ACQUIRED stream with a HardwareBuffer image.

#### Parameters

main

| | |
|---|---|
| stream | The Stream to update. |
| hwbuffer | The HardwareBuffer to acquire. |
| handler | An Executor or Handler to run the release callback on. |
| callback | A callback invoked when the buffer is released by Filament. |

[main]\
open fun [setAcquiredImage](set-acquired-image.md)(stream: [Stream](../../com.google.android.filament/-stream/index.md), hwbuffer: HardwareBuffer, handler: Any, callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html), transform: Array&lt;Float&gt;)

Updates an ACQUIRED stream with a HardwareBuffer image and an optional 3x3 transform matrix.

#### Parameters

main

| | |
|---|---|
| stream | The Stream to update. |
| hwbuffer | The HardwareBuffer to acquire. |
| handler | An Executor or Handler to run the release callback on. |
| callback | A callback invoked when the buffer is released by Filament. |
| transform | An optional 3x3 column-major transform matrix. |
