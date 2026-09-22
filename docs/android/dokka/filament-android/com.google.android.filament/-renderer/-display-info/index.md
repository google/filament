//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[Renderer](../index.md)/[DisplayInfo](index.md)

# DisplayInfo

[main]\
open class [DisplayInfo](index.md)

Use DisplayInfo to set important Display properties. 

This is used to achieve correct frame pacing and dynamic resolution scaling.

## Constructors

| | |
|---|---|
| [DisplayInfo](-display-info.md) | [main]<br>constructor()constructor(refreshRate: Float, presentationDeadlineNanos: Long, vsyncOffsetNanos: Long) |

## Properties

| Name | Summary |
|---|---|
| [presentationDeadlineNanos](presentation-deadline-nanos.md) | [main]<br>open var [presentationDeadlineNanos](presentation-deadline-nanos.md): Long |
| [refreshRate](refresh-rate.md) | [main]<br>open var [refreshRate](refresh-rate.md): Float |
| [vsyncOffsetNanos](vsync-offset-nanos.md) | [main]<br>open var [vsyncOffsetNanos](vsync-offset-nanos.md): Long |

## Functions

| Name | Summary |
|---|---|
| [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md) | [main]<br>open fun [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md)(): Long |
| [getRefreshRate](get-refresh-rate.md) | [main]<br>open fun [getRefreshRate](get-refresh-rate.md)(): Float |
| [getVsyncOffsetNanos](get-vsync-offset-nanos.md) | [main]<br>open fun [getVsyncOffsetNanos](get-vsync-offset-nanos.md)(): Long |
| [setPresentationDeadlineNanos](set-presentation-deadline-nanos.md) | [main]<br>open fun [setPresentationDeadlineNanos](set-presentation-deadline-nanos.md)(presentationDeadlineNanos: Long) |
| [setRefreshRate](set-refresh-rate.md) | [main]<br>open fun [setRefreshRate](set-refresh-rate.md)(refreshRate: Float) |
| [setVsyncOffsetNanos](set-vsync-offset-nanos.md) | [main]<br>open fun [setVsyncOffsetNanos](set-vsync-offset-nanos.md)(vsyncOffsetNanos: Long) |
