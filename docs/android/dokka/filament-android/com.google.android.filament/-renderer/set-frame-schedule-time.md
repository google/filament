//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setFrameScheduleTime](set-frame-schedule-time.md)

# setFrameScheduleTime

[main]\
open fun [setFrameScheduleTime](set-frame-schedule-time.md)(timeSteadyClockNano: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html))

Sets the physical clock time when the frame scheduling callback was entered. This is used by the frame pacer and pipeline estimator to accurately measure the active CPU duration (including app logic running before beginFrame).

#### Parameters

main

| | |
|---|---|
| timeSteadyClockNano | Monotonic steady clock timestamp in nanoseconds since epoch. |
