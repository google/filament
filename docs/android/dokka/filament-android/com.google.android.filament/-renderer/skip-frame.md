//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[skipFrame](skip-frame.md)

# skipFrame

[main]\
open fun [skipFrame](skip-frame.md)(vsyncSteadyClockTimeNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Call skipFrame when momentarily skipping frames, for instance if the content of the scene doesn't change.

#### Parameters

main

| | |
|---|---|
| vsyncSteadyClockTimeNano | The time in nanoseconds when the frame started being rendered, in the [nanoTime](https://developer.android.com/reference/kotlin/java/lang/System.html#nanotime) timebase. Divide this value by 1000000 to convert it to the uptimeMillis time base. This typically comes from android.view.Choreographer.FrameCallback. |
