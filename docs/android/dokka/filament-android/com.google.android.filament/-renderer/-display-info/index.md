//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[DisplayInfo](index.md)

# DisplayInfo

[main]\
open class [DisplayInfo](index.md)

Use DisplayInfo to set important Display properties. 

This is used to achieve correct frame pacing and dynamic resolution scaling.

## Constructors

| | |
|---|---|
| [DisplayInfo](-display-info.md) | [main]<br>constructor()constructor(refreshRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html), presentationDeadlineNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html), vsyncOffsetNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |

## Properties

| Name | Summary |
|---|---|
| [presentationDeadlineNanos](presentation-deadline-nanos.md) | [main]<br>open var [presentationDeadlineNanos](presentation-deadline-nanos.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [refreshRate](refresh-rate.md) | [main]<br>open var [refreshRate](refresh-rate.md): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [vsyncOffsetNanos](vsync-offset-nanos.md) | [main]<br>open var [vsyncOffsetNanos](vsync-offset-nanos.md): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |

## Functions

| Name | Summary |
|---|---|
| [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md) | [main]<br>open fun [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRefreshRate](get-refresh-rate.md) | [main]<br>open fun [getRefreshRate](get-refresh-rate.md)(): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
| [getVsyncOffsetNanos](get-vsync-offset-nanos.md) | [main]<br>open fun [getVsyncOffsetNanos](get-vsync-offset-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [setPresentationDeadlineNanos](set-presentation-deadline-nanos.md) | [main]<br>open fun [setPresentationDeadlineNanos](set-presentation-deadline-nanos.md)(presentationDeadlineNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
| [setRefreshRate](set-refresh-rate.md) | [main]<br>open fun [setRefreshRate](set-refresh-rate.md)(refreshRate: [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html)) |
| [setVsyncOffsetNanos](set-vsync-offset-nanos.md) | [main]<br>open fun [setVsyncOffsetNanos](set-vsync-offset-nanos.md)(vsyncOffsetNanos: [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)) |
