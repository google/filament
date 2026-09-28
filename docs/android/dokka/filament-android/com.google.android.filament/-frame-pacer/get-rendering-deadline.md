//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)/[getRenderingDeadline](get-rendering-deadline.md)

# getRenderingDeadline

[main]\
open fun [getRenderingDeadline](get-rendering-deadline.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Returns the target rendering deadline timepoint computed during the most recent call to `setupFrame()`. 

This timepoint represents the absolute latest point on the steady clock by which the CPU and GPU must complete their rendering operations so that the buffer will successfully meet its display latching window.

#### Return

The upcoming frame's expected rendering deadline on the steady clock.
