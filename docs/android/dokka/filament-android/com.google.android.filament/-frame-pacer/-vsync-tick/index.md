//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[FramePacer](../index.md)/[VsyncTick](index.md)

# VsyncTick

[main]\
open class [VsyncTick](index.md)

Encapsulates VSYNC synchronization telemetry received from the platform compositor.

## Constructors

| | |
|---|---|
| [VsyncTick](-vsync-tick.md) | [main]<br>constructor()constructor(baseTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncPeriod: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), frameScheduleTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), timelines: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[FramePacer.HardwareTimeline](../-hardware-timeline/index.md)&gt;) |

## Properties

| Name | Summary |
|---|---|
| [baseTime](base-time.md) | [main]<br>open var [baseTime](base-time.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [frameScheduleTime](frame-schedule-time.md) | [main]<br>open var [frameScheduleTime](frame-schedule-time.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [vsyncPeriod](vsync-period.md) | [main]<br>open var [vsyncPeriod](vsync-period.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getBaseTime](get-base-time.md) | [main]<br>open fun [getBaseTime](get-base-time.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getFrameScheduleTime](get-frame-schedule-time.md) | [main]<br>open fun [getFrameScheduleTime](get-frame-schedule-time.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getTimelines](get-timelines.md) | [main]<br>open fun [getTimelines](get-timelines.md)(): [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[FramePacer.HardwareTimeline](../-hardware-timeline/index.md)&gt; |
| [getVsyncPeriod](get-vsync-period.md) | [main]<br>open fun [getVsyncPeriod](get-vsync-period.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setBaseTime](set-base-time.md) | [main]<br>open fun [setBaseTime](set-base-time.md)(baseTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setFrameScheduleTime](set-frame-schedule-time.md) | [main]<br>open fun [setFrameScheduleTime](set-frame-schedule-time.md)(frameScheduleTime: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setTimelines](set-timelines.md) | [main]<br>open fun [setTimelines](set-timelines.md)(timelines: [Array](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-array/index.html)&lt;[FramePacer.HardwareTimeline](../-hardware-timeline/index.md)&gt;) |
| [setVsyncPeriod](set-vsync-period.md) | [main]<br>open fun [setVsyncPeriod](set-vsync-period.md)(vsyncPeriod: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
