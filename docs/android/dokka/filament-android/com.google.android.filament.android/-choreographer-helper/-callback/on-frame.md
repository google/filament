//[filament-android](../../../../index.md)/[com.google.android.filament.android](../../index.md)/[ChoreographerHelper](../index.md)/[Callback](index.md)/[onFrame](on-frame.md)

# onFrame

[main]\
abstract fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Called when a new frame should be rendered.

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Monotonic timestamp of the frame in nanoseconds. |

[main]\
open fun [onFrame](on-frame.md)(frameTimeNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), frameData: [Any](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-any/index.html))

Called when a new frame should be rendered, providing optional payload telemetry.

#### Parameters

main

| | |
|---|---|
| frameTimeNanos | Monotonic timestamp of the frame in nanoseconds. |
| frameData | Optional `Choreographer.FrameData` instance on Android 13+ (API 33). Passed as an `Object` to avoid verification failures on older API levels. |
