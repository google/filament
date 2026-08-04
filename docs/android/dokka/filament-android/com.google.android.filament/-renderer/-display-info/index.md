//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[DisplayInfo](index.md)

# DisplayInfo

[main]\
open class [DisplayInfo](index.md)

Information about the display this renderer is associated to

## Constructors

| | |
|---|---|
| [DisplayInfo](-display-info.md) | [main]<br>constructor() |

## Properties

| Name | Summary |
|---|---|
| [presentationDeadlineNanos](presentation-deadline-nanos.md) | [main]<br>open var [~~presentationDeadlineNanos~~](presentation-deadline-nanos.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>How far in advance a buffer must be queued for presentation at a given time in ns On Android you can use getPresentationDeadlineNanos. |
| [refreshRate](refresh-rate.md) | [main]<br>open var [refreshRate](refresh-rate.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)<br>Refresh rate of the display in Hz. |
| [vsyncOffsetNanos](vsync-offset-nanos.md) | [main]<br>open var [~~vsyncOffsetNanos~~](vsync-offset-nanos.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Offset by which vsyncSteadyClockTimeNano provided in beginFrame() is offset in ns On Android you can use getAppVsyncOffsetNanos. |
