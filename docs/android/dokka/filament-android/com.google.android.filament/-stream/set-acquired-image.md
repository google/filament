//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Stream](index.md)/[setAcquiredImage](set-acquired-image.md)

# setAcquiredImage

[main]\
open fun [setAcquiredImage](set-acquired-image.md)(hwbuffer: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), handler: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html), callback: [Runnable](https://developer.android.com/reference/kotlin/java/lang/Runnable.html))

Updates an 

```kotlin
ACQUIRED
```
 stream with an image that is guaranteed to be used in the next frame. This method tells Filament to immediately &quot;acquire&quot; the image and trigger a callback when it is done with it. This should be called by the user outside of beginFrame / endFrame, and should be called only once per frame. If the user pushes images to the same stream multiple times in a single frame, only the final image is honored, but all callbacks are invoked. This method should be called on the same thread that calls [beginFrame](../-renderer/begin-frame.md), which is also where the callback is invoked. This method can only be used for streams that were constructed without calling the Builder.stream method. See [Stream](index.md) for more information about NATIVE and ACQUIRED configurations.

#### Parameters

main

| | |
|---|---|
| hwbuffer | HardwareBuffer (requires API level 26) |
| handler | [Executor](https://developer.android.com/reference/kotlin/java/util/concurrent/Executor.html) or Handler. |
| callback | a callback invoked by `handler` when the `hwbuffer` can be released. |
