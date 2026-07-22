//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[getRenderingDeadlineNanos](get-rendering-deadline-nanos.md)

# getRenderingDeadlineNanos

[main]\
open fun [getRenderingDeadlineNanos](get-rendering-deadline-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Returns the target rendering deadline timestamp computed during the most recent call to setupFrame(). 

This timepoint represents the absolute latest point on the steady clock by which the CPU and GPU must complete their rendering operations so that the buffer will successfully meet its display latching window.

#### Return

The upcoming frame's expected rendering deadline in nanoseconds.
