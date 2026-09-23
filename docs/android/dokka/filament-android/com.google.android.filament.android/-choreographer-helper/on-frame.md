//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[ChoreographerHelper](index.md)/[onFrame](on-frame.md)

# onFrame

[main]\
open fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Base callback invoked when a new frame should be rendered. 

In Inheritance Mode, subclasses should override this method if they do not require `FrameData`. 

In Composition Mode, this delegates to the attached [onFrame](-callback/on-frame.md).

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Monotonic timestamp of the frame in nanoseconds. |

[main]\
open fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), frameData: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html))

Main callback invoked when a new frame should be rendered, providing optional payload telemetry. 

In Inheritance Mode, subclasses can override this method to receive Android 13+ `FrameData`. 

In Composition Mode, this delegates to the attached [onFrame](-callback/on-frame.md).

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Monotonic timestamp of the frame in nanoseconds. |
| frameData | Optional `Choreographer.FrameData` instance on Android 13+ (API 33). Passed as an `Object` to avoid verification failures on older API levels. |
