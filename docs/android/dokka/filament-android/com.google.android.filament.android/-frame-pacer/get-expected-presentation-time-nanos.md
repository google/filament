//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md)

# getExpectedPresentationTimeNanos

[main]\
open fun [getExpectedPresentationTimeNanos](get-expected-presentation-time-nanos.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Returns the target presentation timestamp computed during the most recent call to setupFrame(). 

This timestamp is highly useful for client applications to calculate deterministic, judder-free physics and animation transformations.

#### Return

The upcoming frame's expected presentation timepoint in nanoseconds.
