//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[FramePacer](index.md)/[getExpectedPresentationTime](get-expected-presentation-time.md)

# getExpectedPresentationTime

[main]\
open fun [getExpectedPresentationTime](get-expected-presentation-time.md)(): [Long](https://kotlinlang.org/api/latest/jvm/stdlib/kotlin-stdlib/kotlin/-long/index.html)

Returns the target presentation timepoint computed during the most recent call to `setupFrame()`. 

This timestamp is highly useful for client applications to calculate deterministic, judder-free physics and animation transformations.

#### Return

The upcoming frame's expected presentation timepoint on the steady clock.
