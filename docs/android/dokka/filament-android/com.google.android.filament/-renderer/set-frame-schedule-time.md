//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Renderer](index.md)/[setFrameScheduleTime](set-frame-schedule-time.md)

# setFrameScheduleTime

[main]\
open fun [setFrameScheduleTime](set-frame-schedule-time.md)(time: Long)

Sets the physical clock time when the frame scheduling callback was entered. 

This is used by the frame pacer and pipeline estimator to accurately measure the active CPU duration (including app logic running before beginFrame).

#### Parameters

main

| | |
|---|---|
| time | Monotonic steady clock time_point. |
