//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[DisplayHelper](index.md)

# DisplayHelper

[main]\
open class [DisplayHelper](index.md)

DisplayHelper is here to help managing a Display, for instance being notified when its resolution or refresh rate changes.

## Constructors

| | |
|---|---|
| [DisplayHelper](-display-helper.md) | [main]<br>constructor(context: Context)<br>Creates a DisplayHelper which helps managing a Display.<br>constructor(context: Context, handler: Handler)<br>Creates a DisplayHelper which helps manage a Display and provides a Handler where callbacks can execute filament code. |

## Functions

| Name | Summary |
|---|---|
| [attach](attach.md) | [main]<br>open fun [attach](attach.md)(renderer: [Renderer](../../com.google.android.filament/-renderer/index.md), display: Display)<br>Sets the filament [Renderer](../../com.google.android.filament/-renderer/index.md) associated to the Display, from this point on, [Renderer.DisplayInfo](../../com.google.android.filament/-renderer/-display-info/index.md) will be automatically updated when the Display properties change. |
| [detach](detach.md) | [main]<br>open fun [detach](detach.md)()<br>Disconnect the previously set [Renderer](../../com.google.android.filament/-renderer/index.md) from Display This is typically called from [onDetachedFromSurface](../-ui-helper/-renderer-callback/on-detached-from-surface.md). |
| [getAppVsyncOffsetNanos](get-app-vsync-offset-nanos.md) | [main]<br>open fun [getAppVsyncOffsetNanos](get-app-vsync-offset-nanos.md)(display: Display): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getDisplay](get-display.md) | [main]<br>open fun [getDisplay](get-display.md)(): Display<br>Returns the Display currently monitored |
| [getDisplayInfo](get-display-info.md) | [main]<br>open fun [getDisplayInfo](get-display-info.md)(display: Display, info: [Renderer.DisplayInfo](../../com.google.android.filament/-renderer/-display-info/index.md)): [Renderer.DisplayInfo](../../com.google.android.filament/-renderer/-display-info/index.md)<br>Populate a [Renderer.DisplayInfo](../../com.google.android.filament/-renderer/-display-info/index.md) with properties from the given Display |
| [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md) | [main]<br>open fun [getPresentationDeadlineNanos](get-presentation-deadline-nanos.md)(display: Display): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html) |
| [getRefreshPeriodNanos](get-refresh-period-nanos.md) | [main]<br>open fun [getRefreshPeriodNanos](get-refresh-period-nanos.md)(display: Display): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)<br>Returns a Display's refresh period in nanoseconds |
| [getRefreshRate](get-refresh-rate.md) | [main]<br>open fun [getRefreshRate](get-refresh-rate.md)(display: Display): [Float](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-float/index.html) |
